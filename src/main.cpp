
#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

class $modify(UGTMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init())
            return false;

        auto size = CCDirector::sharedDirector()->getWinSize();

        // Main UGT button
        auto sprite = ButtonSprite::create(
            "UGT", "goldFont.fnt", "GJ_button_01.png", 0.8f
        );

        if (!sprite)
            return true;

        auto theme = Mod::get()->getSettingValue<std::string>("menu-theme");

        ccColor3B color = {0, 220, 255};
        if (theme == "Purple") color = {190, 100, 255};
        if (theme == "Green")  color = {80, 255, 130};
        if (theme == "Orange") color = {255, 160, 50};
        if (theme == "Classic") color = {255, 255, 255};

        sprite->setColor(color);

        auto button = CCMenuItemSpriteExtra::create(
            sprite, this,
            menu_selector(UGTMenuLayer::onToolkitButton)
        );

        auto mainMenu = CCMenu::create();
        if (!mainMenu || !button)
            return true;

        mainMenu->addChild(button);
        mainMenu->setPosition({60.f, size.height / 2.f});
        this->addChild(mainMenu, 100);

        // Toolkit sections
        auto panel = CCMenu::create();
        if (!panel)
            return true;

        panel->setTag(7721);
        panel->setPosition({60.f, size.height / 2.f});
        panel->setVisible(false);

        auto addTab = [panel](
            const char* name,
            float y,
            SEL_MenuHandler callback,
            CCNode* target
        ) {
            auto tabSprite = ButtonSprite::create(
                name, "goldFont.fnt", "GJ_button_02.png", 0.65f
            );
            if (!tabSprite)
                return;

            auto item = CCMenuItemSpriteExtra::create(
                tabSprite, target, callback
            );
            if (!item)
                return;

            item->setPosition({125.f, y});
            panel->addChild(item);
        };

        addTab("Macro Lab",      90.f,
            menu_selector(UGTMenuLayer::onMacroTab), this);
        addTab("Pathfinder",     45.f,
            menu_selector(UGTMenuLayer::onPathfinderTab), this);
        addTab("Frame Counter",   0.f,
            menu_selector(UGTMenuLayer::onFrameTab), this);
        addTab("Creator Tools", -45.f,
            menu_selector(UGTMenuLayer::onCreatorTab), this);
        addTab("Settings",      -90.f,
            menu_selector(UGTMenuLayer::onSettingsTab), this);

        this->addChild(panel, 100);
        return true;
    }

    void onToolkitButton(CCObject*) {
        auto panel = this->getChildByTag(7721);
        if (panel)
            panel->setVisible(!panel->isVisible());
    }

    void onMacroTab(CCObject*) {
        FLAlertLayer::create(
            "UGT - Macro Lab",
            "Native format: .sm\n\n"
            "Planned tools:\n"
            "- Record and stop gameplay input\n"
            "- Save and load .sm files\n"
            "- Playback and replay controls\n\n"
            "Recording and playback are not implemented yet.",
            "Close"
        )->show();
    }

    void onPathfinderTab(CCObject*) {
        FLAlertLayer::create(
            "UGT - Pathfinder",
            "Level analysis and route planning.\n\n"
            "Planned tools:\n"
            "- Route and obstacle analysis\n"
            "- Timing and path visualization\n\n"
            "Pathfinding is not implemented yet.",
            "Close"
        )->show();
    }

    void onFrameTab(CCObject*) {
        FLAlertLayer::create(
            "UGT - Frame Counter",
            "Planned tools:\n"
            "- Frame and timing display\n"
            "- Custom window settings\n"
            "- Optional sound alerts\n\n"
            "The live frame counter is not implemented yet.",
            "Close"
        )->show();
    }

    void onCreatorTab(CCObject*) {
        FLAlertLayer::create(
            "UGT - Creator Tools",
            "Creator toolkit roadmap:\n"
            "- Level-building utilities\n"
            "- Decoration helpers\n"
            "- Creator workflow tools\n\n"
            "These tools are still in development.",
            "Close"
        )->show();
    }

    void onSettingsTab(CCObject*) {
        auto theme = Mod::get()->getSettingValue<std::string>("menu-theme");

        FLAlertLayer::create(
            "UGT - Settings",
            "Current theme: " + theme + "\n\n"
            "To change the theme, open Geode's mod settings "
            "and select Ultimate GD Toolkit.\n\n"
            "Available themes: Neon, Purple, Green, Orange, Classic.",
            "Close"
        )->show();
    }
};
