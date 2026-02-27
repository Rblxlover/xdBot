#pragma once
#include "../includes.hpp"

class SaveMacroLayer : public geode::Popup {

    TextInput* authorInput = nullptr;
    TextInput* descInput = nullptr;
    TextInput* nameInput = nullptr;

    CCMenuItemToggler* gdr1Toggle = nullptr;
    CCMenuItemToggler* jsonToggle = nullptr;

    SaveFormat selectedFormat = SaveFormat::GDR2;
    SaveFormat defaultFormat = SaveFormat::GDR2;

private:

    bool init() override {
        std::string defaultFormatStr = Mod::get()->getSettingValueString("default_save_format");
        if (defaultFormatStr == "GDR") {
            defaultFormat = SaveFormat::GDR1;
            selectedFormat = SaveFormat::GDR1;
        } else if (defaultFormatStr == "JSON") {
            defaultFormat = SaveFormat::JSON;
            selectedFormat = SaveFormat::JSON;
        } else {
            defaultFormat = SaveFormat::GDR2;
            selectedFormat = SaveFormat::GDR2;
        }

        if (!Popup::init(285, 210, Utils::getTexture().c_str())) return false;
        Utils::setBackgroundColor(m_bgSprite);

        setTitle("Save Macro");

        cocos2d::CCPoint offset = (CCDirector::sharedDirector()->getWinSize() - m_mainLayer->getContentSize()) / 2;
        m_mainLayer->setPosition(m_mainLayer->getPosition() - offset);
        m_closeBtn->setPosition(m_closeBtn->getPosition() + offset);
        m_bgSprite->setPosition(m_bgSprite->getPosition() + offset);
        m_title->setPosition(m_title->getPosition() + offset);

        CCMenu* menu = CCMenu::create();
        m_mainLayer->addChild(menu);

        authorInput = TextInput::create(104, "Author", "chatFont.fnt");
        authorInput->setPosition({ 61, 50 });
        authorInput->setString(GJAccountManager::sharedState()->m_username.c_str());
        menu->addChild(authorInput);

        CCLabelBMFont* lbl = CCLabelBMFont::create("(optional)", "chatFont.fnt");
        lbl->setPosition({ 61, 28 });
        lbl->setOpacity(73);
        lbl->setScale(0.575);
        menu->addChild(lbl);

        nameInput = TextInput::create(104, "Name", "chatFont.fnt");
        nameInput->setPosition({ -61, 50 });

        nameInput->setString(Global::get().macro.levelInfo.name);

        menu->addChild(nameInput);

        descInput = TextInput::create(226, "Description (optional)", "chatFont.fnt");
        descInput->setPositionY(0);
        menu->addChild(descInput);

        ButtonSprite* spr = ButtonSprite::create("Save");
        spr->setScale(0.725f);
        CCMenuItemSpriteExtra* btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(SaveMacroLayer::onSave));
        btn->setPositionY(-48);
        menu->addChild(btn);

        CCSprite* spriteOn = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
        CCSprite* spriteOff = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
        gdr1Toggle = CCMenuItemToggler::create(spriteOff, spriteOn, this, menu_selector(SaveMacroLayer::onGDR1Toggle));
        gdr1Toggle->setPosition({ -124, -86 });
        gdr1Toggle->setScale(0.525);
        menu->addChild(gdr1Toggle);

        lbl = CCLabelBMFont::create("GDR", "bigFont.fnt");
        lbl->setPosition({ -97, -85.5 });
        lbl->setScale(0.35);
        menu->addChild(lbl);

        spriteOn = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
        spriteOff = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
        jsonToggle = CCMenuItemToggler::create(spriteOff, spriteOn, this, menu_selector(SaveMacroLayer::onJSONToggle));
        jsonToggle->setPosition({ -50, -86 });
        jsonToggle->setScale(0.525);
        menu->addChild(jsonToggle);

        lbl = CCLabelBMFont::create("JSON", "bigFont.fnt");
        lbl->setPosition({ -23, -85.5 });
        lbl->setScale(0.35);
        menu->addChild(lbl);

        if (selectedFormat == SaveFormat::GDR1) {
            gdr1Toggle->toggle(true);
        } else if (selectedFormat == SaveFormat::JSON) {
            jsonToggle->toggle(true);
        }

        lbl = CCLabelBMFont::create("Default: GDR2", "chatFont.fnt");
        lbl->setPosition({ 110, -85.5 });
        lbl->setScale(0.5);
        lbl->setOpacity(100);
        lbl->setID("format-label");
        menu->addChild(lbl);

        return true;
    }

    void updateFormatLabel() {
        CCLabelBMFont* lbl = static_cast<CCLabelBMFont*>(m_mainLayer->getChildByType<CCMenu>(0)->getChildByID("format-label"));
        if (!lbl) return;

        bool isDefault = (selectedFormat == defaultFormat);
        const char* prefix = isDefault ? "Default" : "Format";

        switch (selectedFormat) {
            case SaveFormat::GDR2: lbl->setString(fmt::format("{}: GDR2", prefix).c_str()); break;
            case SaveFormat::GDR1: lbl->setString(fmt::format("{}: GDR1", prefix).c_str()); break;
            case SaveFormat::JSON: lbl->setString(fmt::format("{}: JSON", prefix).c_str()); break;
        }
    }

    void onGDR1Toggle(CCObject*) {
        bool toggled = !gdr1Toggle->isToggled();
        if (toggled) {
            selectedFormat = SaveFormat::GDR1;
            jsonToggle->toggle(false);
        } else {
            selectedFormat = defaultFormat;
        }
        updateFormatLabel();
    }

    void onJSONToggle(CCObject*) {
        bool toggled = !jsonToggle->isToggled();
        if (toggled) {
            selectedFormat = SaveFormat::JSON;
            gdr1Toggle->toggle(false);
        } else {
            selectedFormat = defaultFormat;
        }
        updateFormatLabel();
    }

public:

    STATIC_CREATE(SaveMacroLayer)
    
    static void open() {
        if (Global::get().macro.inputs.empty())
            return FLAlertLayer::create("Save Macro", "You can't save an <cl>empty</c> macro.", "OK")->show();

        #ifdef GEODE_IS_IOS
        std::filesystem::path path = Mod::get()->getSaveDir() / "macros";
        #else
        std::filesystem::path path = Mod::get()->getSettingValue<std::filesystem::path>("macros_folder");
        #endif

        if (!std::filesystem::exists(path)) {
            if (!utils::file::createDirectoryAll(path).isOk())
                return FLAlertLayer::create("Error", ("There was an error getting the folder \"" + geode::utils::string::pathToString(path) + "\". ID: 10").c_str(), "OK")->show();
        }

        SaveMacroLayer* layerReal = create();
        layerReal->m_noElasticity = true;
        layerReal->show();
    }

    void onSave(CCObject*) {
        std::string macroName = nameInput->getString();
        if (macroName == "")
            return FLAlertLayer::create("Save Macro", "Give a <cl>name</c> to the macro.", "OK")->show();

        #ifdef GEODE_IS_IOS
        std::filesystem::path path = Mod::get()->getSaveDir() / "macros" / macroName;
        #else
        std::filesystem::path path = Mod::get()->getSettingValue<std::filesystem::path>("macros_folder") / macroName;
        #endif
        std::string author = authorInput->getString();
        std::string desc = descInput->getString();

        int result = Macro::save(author, desc, geode::utils::string::pathToString(path), selectedFormat);

        if (result != 0)
            return FLAlertLayer::create("Error", "There was an error saving the macro. ID: " + geode::utils::numToString(result), "OK")->show();

        this->keyBackClicked();
        Notification::create("Macro Saved", NotificationIcon::Success)->show();
    }

};
