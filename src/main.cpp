
#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PauseLayer.hpp>

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

using namespace geode::prelude;

// ============================================================
// ULTIMATE GD TOOLKIT
// Phase 1: Native .sm macro recording and playback
// ============================================================

namespace UGT {
    struct MacroEvent {
        std::uint64_t frame;
        int button;
        bool pressed;
        bool player2;
    };

    static std::vector<MacroEvent> events;
    static std::uint64_t frame = 0;
    static std::size_t playbackIndex = 0;

    static bool recording = false;
    static bool playing = false;
    static bool injectingInput = false;

    static constexpr std::size_t MAX_EVENTS = 2000000;

    // Native macro file: latest.sm
    static std::filesystem::path macroPath() {
        auto dir = Mod::get()->getSaveDir() / "macros";
        std::error_code ec;
        std::filesystem::create_directories(dir, ec);
        return dir / "latest.sm";
    }

    static void message(
        const char* title,
        const std::string& text
    ) {
        auto alert = FLAlertLayer::create(title, text, "Close");
        if (alert) alert->show();
    }

    // UGT_SM version 1 file format:
    // UGT_SM 1 <event-count>
    // <frame> <button> <pressed> <player2>
    static bool saveMacro() {
        if (events.empty()) return false;

        std::ofstream file(macroPath());
        if (!file.is_open()) return false;

        file << "UGT_SM 1 " << events.size() << '\n';

        for (auto const& event : events) {
            file << event.frame << ' '
                 << event.button << ' '
                 << (event.pressed ? 1 : 0) << ' '
                 << (event.player2 ? 1 : 0) << '\n';
        }

        file.flush();
        return file.good();
    }

    static bool loadMacro() {
        std::ifstream file(macroPath());
        if (!file.is_open()) return false;

        std::string magic;
        int version = 0;
        std::uint64_t count = 0;

        if (!(file >> magic >> version >> count) ||
            magic != "UGT_SM" ||
            version != 1 ||
            count == 0 ||
            count > MAX_EVENTS) {
            return false;
        }

        std::vector<MacroEvent> loaded;
        loaded.reserve(static_cast<std::size_t>(count));

        std::uint64_t previousFrame = 0;

        for (std::uint64_t i = 0; i < count; ++i) {
            std::uint64_t f;
            int button, pressed, player2;

            if (!(file >> f >> button >> pressed >> player2))
                return false;

            if (button < 0 || button > 255 ||
                (pressed != 0 && pressed != 1) ||
                (player2 != 0 && player2 != 1) ||
                (i > 0 && f < previousFrame)) {
                return false;
            }

            loaded.push_back({
                f, button, pressed == 1, player2 == 1
            });

            previousFrame = f;
        }

        events = std::move(loaded);
        playbackIndex = 0;
        return true;
    }

    static void recordInput(
        PlayerObject* player,
        PlayerButton button,
        bool pressed
    ) {
        if (!recording || injectingInput || !player)
            return;

        auto* layer = PlayLayer::get();
        if (!layer) return;

        bool player2 = player == layer->m_player2;

        if (player != layer->m_player1 && !player2)
            return;

        events.push_back({
            frame,
            static_cast<int>(button),
            pressed,
            player2
        });
    }

    static void applyEvent(MacroEvent const& event) {
        auto* layer = PlayLayer::get();
        if (!layer) return;

        auto* player = event.player2
            ? layer->m_player2
            : layer->m_player1;

        if (!player) return;

        auto button = static_cast<PlayerButton>(event.button);

        injectingInput = true;

        if (event.pressed)
            player->pushButton(button);
        else
            player->releaseButton(button);

        injectingInput = false;
    }

    static void stopPlayback() {
        playing = false;
        playbackIndex = 0;

        if (auto* layer = PlayLayer::get()) {
            injectingInput = true;

            if (layer->m_player1)
                layer->m_player1->releaseAllButtons();

            if (layer->m_player2)
                layer->m_player2->releaseAllButtons();

            injectingInput = false;
        }
    }
}

// ============================================================
// PLAYER INPUT HOOKS
// ============================================================

class $modify(UGTPlayerObject, PlayerObject) {
    bool pushButton(PlayerButton button) {
        if (UGT::playing && !UGT::injectingInput)
            return false;

        bool result = PlayerObject::pushButton(button);

        UGT::recordInput(this, button, true);
        return result;
    }

    bool releaseButton(PlayerButton button) {
        if (UGT::playing && !UGT::injectingInput)
            return false;

        bool result = PlayerObject::releaseButton(button);

        UGT::recordInput(this, button, false);
        return result;
    }
};

// ============================================================
// GAMEPLAY FRAME TRACKING AND PLAYBACK
// ============================================================

class $modify(UGTPlayLayer, PlayLayer) {
    bool init(
        GJGameLevel* level,
        bool useReplay,
        bool dontCreateObjects
    ) {
        if (!PlayLayer::init(
            level, useReplay, dontCreateObjects
        )) {
            return false;
        }

        UGT::frame = 0;
        UGT::playbackIndex = 0;

        return true;
    }

    void update(float dt) {
        if (UGT::playing) {
            while (
                UGT::playbackIndex < UGT::events.size() &&
                UGT::events[UGT::playbackIndex].frame <= UGT::frame
            ) {
                UGT::applyEvent(
                    UGT::events[UGT::playbackIndex]
                );

                ++UGT::playbackIndex;
            }

            if (UGT::playbackIndex >= UGT::events.size()) {
                UGT::stopPlayback();
            }
        }

        PlayLayer::update(dt);
        ++UGT::frame;
    }

    void resetLevel() {
        if (UGT::recording)
            UGT::recording = false;

        PlayLayer::resetLevel();

        UGT::frame = 0;
        UGT::playbackIndex = 0;
    }

    void onQuit() {
        UGT::recording = false;
        UGT::stopPlayback();

        PlayLayer::onQuit();
    }
};

// ============================================================
// PAUSE MENU CONTROLS
// ============================================================

class $modify(UGTPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto win = CCDirector::sharedDirector()->getWinSize();

        auto menu = CCMenu::create();
        if (!menu) return;

        menu->setPosition({
            win.width * 0.5f,
            32.f
        });

        auto addButton = [&](const char* label,
                             SEL_MenuHandler callback) {
            auto sprite = ButtonSprite::create(
                label,
                "goldFont.fnt",
                "GJ_button_02.png",
                0.65f
            );

            if (!sprite) return;

            auto item = CCMenuItemSpriteExtra::create(
                sprite, this, callback
            );

            if (item) {
                item->setScale(0.7f);
                menu->addChild(item);
            }
        };

        addButton(
            "REC / STOP",
            menu_selector(UGTPauseLayer::onRecord)
        );

        addButton(
            "SAVE .SM",
            menu_selector(UGTPauseLayer::onSave)
        );

        addButton(
            "LOAD .SM",
            menu_selector(UGTPauseLayer::onLoad)
        );

        addButton(
            "PLAY / STOP",
            menu_selector(UGTPauseLayer::onPlayback)
        );

        auto children = menu->getChildren();

        if (children && children->count() == 4) {
            static_cast<CCNode*>(children->objectAtIndex(0))
                ->setPosition({-132.f, 0.f});

            static_cast<CCNode*>(children->objectAtIndex(1))
                ->setPosition({-44.f, 0.f});

            static_cast<CCNode*>(children->objectAtIndex(2))
                ->setPosition({44.f, 0.f});

            static_cast<CCNode*>(children->objectAtIndex(3))
                ->setPosition({132.f, 0.f});
        }

        this->addChild(menu, 100);
    }

    void onRecord(CCObject*) {
        if (UGT::playing) {
            UGT::message(
                "UGT Macro Lab",
                "Stop playback before recording."
            );
            return;
        }

        if (!UGT::recording) {
            UGT::events.clear();
            UGT::recording = true;

            UGT::message(
                "UGT",
                "Recording started. Play, pause, then save "
                "your recording as a .sm macro."
            );
        } else {
            UGT::recording = false;

            UGT::message(
                "UGT",
                fmt::format(
                    "Recording stopped. Captured {} events.",
                    UGT::events.size()
                )
            );
        }
    }

    void onSave(CCObject*) {
        UGT::recording = false;

        if (!UGT::saveMacro()) {
            UGT::message(
                "UGT",
                "Save failed. Record some input events first."
            );
            return;
        }

        UGT::message(
            "UGT",
            "Saved native .sm macro to the mod's macros folder."
        );
    }

    void onLoad(CCObject*) {
        if (UGT::recording || UGT::playing) {
            UGT::message(
                "UGT",
                "Stop recording or playback before loading."
            );
            return;
        }

        if (!UGT::loadMacro()) {
            UGT::message(
                "UGT",
                "No valid latest.sm file found. Record and save "
                "a macro first."
            );
            return;
        }

        UGT::message(
            "UGT",
            fmt::format(
                "Loaded {} input events.",
                UGT::events.size()
            )
        );
    }

    void onPlayback(CCObject*) {
        if (UGT::playing) {
            UGT::stopPlayback();
            UGT::message("UGT", "Playback stopped.");
            return;
        }

        if (UGT::recording) {
            UGT::message(
                "UGT",
                "Stop recording before playback."
            );
            return;
        }

        if (UGT::events.empty() && !UGT::loadMacro()) {
            UGT::message(
                "UGT",
                "No macro loaded. Record and save first."
            );
            return;
        }

        auto* layer = PlayLayer::get();
        if (!layer) return;

        // Restart to align the recorded frame timeline.
        layer->resetLevel();

        UGT::frame = 0;
        UGT::playbackIndex = 0;
        UGT::playing = true;

        UGT::message(
            "UGT",
            "Playback is armed. Close this message and resume "
            "the level. Test in Practice Mode first."
        );
    }
};

// ============================================================
// MAIN MENU: minimal toolkit entry point
// ============================================================

class $modify(UGTMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init())
            return false;

        auto win = CCDirector::sharedDirector()->getWinSize();
        auto sprite = ButtonSprite::create(
            "Ultimate GD Toolkit",
            "goldFont.fnt",
            "GJ_button_02.png",
            0.7f
        );

        if (!sprite) return true;

        auto item = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(UGTMenuLayer::onToolkit)
        );

        auto menu = CCMenu::create();
        if (!item || !menu) return true;

        menu->addChild(item);
        menu->setPosition({
            win.width * 0.5f,
            30.f
        });

        this->addChild(menu, 100);
        return true;
    }

    void onToolkit(CCObject*) {
        UGT::message(
            "Ultimate GD Toolkit",
            "Phase 1: native .sm macro recording, save/load, "
            "and playback controls in the pause menu.\\n\\n"
            "Pathfinder, advanced frame timing, speed/music sync, "
            "and automatic decoration are future phases."
        );
    }
};
