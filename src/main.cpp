
#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

class $modify(UGTMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init())
            return false;

        auto theme = Mod::get()->getSettingValue<std::string>("menu-theme");

        ccColor3B themeColor = {0, 220, 255};

        if (theme == "Purple")
            themeColor = {190, 100, 255};
        else if (theme == "Green")
            themeColor = {80, 255, 130};
        else if (theme == "Orange")
            themeColor = {255, 160, 50};
        else if (theme == "Classic")
            themeColor = {255, 255, 255};

        auto sprite = ButtonSprite::create(
            "UGT",
            "goldFont.fnt",
            "GJ_button_01.png",
            0.8f
        );

        if (!sprite)
            return true;

        sprite->setColor(themeColor);

        auto button = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(UGTMenuLayer::onToolkitButton)
        );

        auto menu = CCMenu::create();
        if (!menu || !button)
            return true;

        menu->addChild(button);
        menu->setPosition({60.f, CCDirector::sharedDirector()
            ->getWinSize().height / 2.f});

        this->addChild(menu, 100);
        return true;
    }

    void onToolkitButton(CCObject*) {
        auto theme = Mod::get()->getSettingValue<std::string>("menu-theme");

        FLAlertLayer::create(
            "Ultimate GD Toolkit",
            "Welcome to the Toolkit!\\n\\n"
            "Menu button: WORKING\\n"
            "Theme setting: " + theme + "\\n\\n"
            "Macro Recorder: Coming Soon\\n"
            "Pathfinder: Coming Soon\\n"
            "Frame Counter: Coming Soon\\n"
            "Creator Tools: Coming Soon",
            "Close"
        )->show();
    }
};
