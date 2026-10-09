
#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/LevelSelectLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

// ============================================================
// ULTIMATE GD TOOLKIT
// Floating menu, custom logo, level-select Pathfinder entry,
// theme colors, and a gameplay update-frame counter.
// ============================================================

namespace UGT {
    constexpr int MENU_TAG = 7721;
    constexpr int FRAME_LABEL_TAG = 7722;

    static ccColor3B getThemeColor() {
        auto theme =
            Mod::get()->getSettingValue<std::string>("menu-theme");

        if (theme == "Purple")
            return {190, 100, 255};

        if (theme == "Green")
            return {80, 255, 130};

        if (theme == "Orange")
            return {255, 160, 50};

        if (theme == "Classic")
            return {255, 255, 255};

        return {0, 220, 255};
    }

    static void showMessage(
        const char* title,
        const std::string& message
    ) {
        FLAlertLayer::create(
            title,
            message,
            "Close"
        )->show();
    }

    static CCNode* createLogo(float maxWidth) {
        // logo.png must be included in the mod resources.
        auto logo = CCSprite::create("logo.png"_spr);

        if (!logo)
            return nullptr;

        auto size = logo->getContentSize();

        if (size.width > 0.f && size.height > 0.f) {
            float scale = maxWidth / size.width;

            if (scale > 1.f)
                scale = 1.f;

            logo->setScale(scale);
        }

        return logo;
    }

    static CCMenuItemSpriteExtra* createTextButton(
        const char* text,
        SEL_MenuHandler callback,
        CCObject* target
    ) {
        auto sprite = ButtonSprite::create(
            text,
            "goldFont.fnt",
            "GJ_button_02.png",
            0.65f
        );

        if (!sprite)
            return nullptr;

        sprite->setColor(getThemeColor());

        return CCMenuItemSpriteExtra::create(
            sprite,
            target,
            callback
        );
    }
}

// ============================================================
// MAIN MENU: floating logo button and toolkit panel
// ============================================================

class $modify(UGTMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init())
            return false;

        auto winSize =
            CCDirector::sharedDirector()->getWinSize();

        auto menu = CCMenu::create();

        if (!menu)
            return true;

        // Use the user's logo as the floating menu button.
        auto logo = UGT::createLogo(64.f);

        if (!logo) {
            log::error(
                "UGT logo.png could not be loaded. "
                "Check the resources configuration."
            );

            return true;
        }

        auto button = CCMenuItemSpriteExtra::create(
            logo,
            this,
            menu_selector(UGTMenuLayer::onToolkitButton)
        );

        if (!button)
            return true;

        menu->addChild(button);

        // Left side of the screen.
        menu->setPosition({
            48.f,
            winSize.height * 0.55f
        });

        this->addChild(menu, 100);

        // Create the toolkit panel.
        auto panel = CCNode::create();

        if (!panel)
            return true;

        panel->setTag(UGT::MENU_TAG);
        panel->setContentSize({300.f, 270.f});
        panel->setAnchorPoint({0.5f, 0.5f});
        panel->setPosition({
            winSize.width * 0.5f,
            winSize.height * 0.5f
        });
        panel->setVisible(false);

        // Dark translucent background.
        auto background = CCLayerColor::create(
            {15, 20, 35, 235},
            300.f,
            270.f
        );

        if (background) {
            background->setAnchorPoint({0.5f, 0.5f});
            background->setPosition({0.f, 0.f});
            panel->addChild(background);
        }

        // Logo at the top of the opened panel.
        auto panelLogo = UGT::createLogo(100.f);

        if (panelLogo) {
            panelLogo->setPosition({150.f, 232.f});
            panel->addChild(panelLogo);
        }

        // Toolkit title.
        auto title = CCLabelBMFont::create(
            "ULTIMATE GD TOOLKIT",
            "goldFont.fnt"
        );

        if (title) {
            title->setScale(0.48f);
            title->setColor(UGT::getThemeColor());
            title->setPosition({150.f, 205.f});
            panel->addChild(title);
        }

        // Toolkit section buttons.
        auto macro = UGT::createTextButton(
            "Macro Lab",
            menu_selector(UGTMenuLayer::onMacroTab),
            this
        );

        auto pathfinder = UGT::createTextButton(
            "Pathfinder",
            menu_selector(UGTMenuLayer::onPathfinderTab),
            this
        );

        auto frame = UGT::createTextButton(
            "Frame Counter",
            menu_selector(UGTMenuLayer::onFrameTab),
            this
        );

        auto creator = UGT::createTextButton(
            "Creator Tools",
            menu_selector(UGTMenuLayer::onCreatorTab),
            this
        );

        auto settings = UGT::createTextButton(
            "Settings",
            menu_selector(UGTMenuLayer::onSettingsTab),
            this
        );

        if (macro) {
            macro->setPosition({150.f, 168.f});
            panel->addChild(macro);
        }

        if (pathfinder) {
            pathfinder->setPosition({150.f, 130.f});
            panel->addChild(pathfinder);
        }

        if (frame) {
            frame->setPosition({150.f, 92.f});
            panel->addChild(frame);
        }

        if (creator) {
            creator->setPosition({150.f, 54.f});
            panel->addChild(creator);
        }

        if (settings) {
            settings->setPosition({150.f, 16.f});
            panel->addChild(settings);
        }

        auto closeButton = CCMenuItemSpriteExtra::create(
            ButtonSprite::create(
                "X",
                "goldFont.fnt",
                "GJ_button_01.png",
                0.65f
            ),
            this,
            menu_selector(UGTMenuLayer::onClosePanel)
        );

        if (closeButton) {
            auto closeMenu = CCMenu::create();
            closeMenu->addChild(closeButton);
            closeMenu->setPosition({280.f, 250.f});
            panel->addChild(closeMenu);
        }

        this->addChild(panel, 101);

        return true;
    }

    void onToolkitButton(CCObject*) {
        auto panel = this->getChildByTag(UGT::MENU_TAG);

        if (panel)
            panel->setVisible(!panel->isVisible());
    }

    void onClosePanel(CCObject*) {
        auto panel = this->getChildByTag(UGT::MENU_TAG);

        if (panel)
            panel->setVisible(false);
    }

    void onMacroTab(CCObject*) {
        UGT::showMessage(
            "UGT - Macro Lab",
            "Native format: .sm\n\n"
            "Planned features:\n"
            "- Record and stop inputs\n"
            "- Save and load .sm files\n"
            "- Playback controls\n\n"
            "Recording and playback are not implemented yet."
        );
    }

    void onPathfinderTab(CCObject*) {
        UGT::showMessage(
            "UGT - Pathfinder",
            "Pathfinder menu\n\n"
            "Planned features:\n"
            "- Route planning\n"
            "- Obstacle analysis\n"
            "- Timing visualization\n\n"
            "The pathfinding engine is not implemented yet."
        );
    }

    void onFrameTab(CCObject*) {
        UGT::showMessage(
            "UGT - Frame Counter",
            "The gameplay update-frame counter can be enabled "
            "in Geode mod settings.\n\n"
            "Note: update frames are not the same as "
            "precisely measured rendered frames."
        );
    }

    void onCreatorTab(CCObject*) {
        UGT::showMessage(
            "UGT - Creator Tools",
            "Creator Tools roadmap\n\n"
            "- Level-building utilities\n"
            "- Decoration helpers\n"
            "- Creator workflow tools\n\n"
            "These tools are still in development."
        );
    }

    void onSettingsTab(CCObject*) {
        auto theme =
            Mod::get()->getSettingValue<std::string>("menu-theme");

        UGT::showMessage(
            "UGT - Settings",
            "Current theme: " + theme +
            "\n\nChange the theme in Geode's settings "
            "for Ultimate GD Toolkit.\n\n"
            "Available themes: Neon, Purple, Green, "
            "Orange, Classic."
        );
    }
};

// ============================================================
// LEVEL SELECTION: add a dedicated Pathfinder entry
// ============================================================

class $modify(UGTLevelSelectLayer, LevelSelectLayer) {
    bool init(int page) {
        if (!LevelSelectLayer::init(page))
            return false;

        auto winSize =
            CCDirector::sharedDirector()->getWinSize();

        auto menu = CCMenu::create();

        if (!menu)
            return true;

        auto button = UGT::createTextButton(
            "UGT Pathfinder",
            menu_selector(UGTLevelSelectLayer::onPathfinder),
            this
        );

        if (!button)
            return true;

        menu->addChild(button);

        // Keep the button near the left edge.
        menu->setPosition({
            78.f,
            winSize.height * 0.35f
        });

        this->addChild(menu, 100);

        return true;
    }

    void onPathfinder(CCObject*) {
        UGT::showMessage(
            "UGT - Pathfinder",
            "Level Selection Pathfinder\n\n"
            "This is the entry point for the planned "
            "level route-analysis tools.\n\n"
            "Route calculation and obstacle analysis "
            "are not implemented yet."
        );
    }
};

// ============================================================
// GAMEPLAY: optional update-frame counter
// ============================================================

class $modify(UGTPlayLayer, PlayLayer) {
    bool init(
        GJGameLevel* level,
        bool useReplay,
        bool dontCreateObjects
    ) {
        if (!PlayLayer::init(
            level,
            useReplay,
            dontCreateObjects
        )) {
            return false;
        }

        if (!Mod::get()->getSettingValue<bool>(
            "frame-counter-enabled"
        )) {
            return true;
        }

        auto winSize =
            CCDirector::sharedDirector()->getWinSize();

        auto label = CCLabelBMFont::create(
            "Updates: 0",
            "bigFont.fnt"
        );

        if (!label)
            return true;

        label->setTag(UGT::FRAME_LABEL_TAG);
        label->setScale(0.45f);
        label->setAnchorPoint({0.f, 1.f});
        label->setPosition({
            12.f,
            winSize.height - 12.f
        });
        label->setColor(UGT::getThemeColor());

        this->addChild(label, 1000);

        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);

        auto label =
            this->getChildByTag(UGT::FRAME_LABEL_TAG);

        if (!label)
            return;

        // Counts gameplay update calls, not hardware-rendered frames.
        ++m_ugtUpdateCount;

        auto text = CCString::createWithFormat(
            "Updates: %u",
            m_ugtUpdateCount
        );

        if (text) {
            static_cast<CCLabelBMFont*>(label)->setString(
                text->getCString()
            );
        }
    }

    unsigned int m_ugtUpdateCount = 0;
};
