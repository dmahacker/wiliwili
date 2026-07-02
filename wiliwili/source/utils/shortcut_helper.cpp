//
// Created by fang on 2025/7/27.
//

#include "utils/shortcut_helper.hpp"

#include <algorithm>
#include <unordered_map>
#include <utility>
#include <vector>
#include <pystring.h>

#include "borealis/core/logger.hpp"
#include "borealis/core/view.hpp"

static std::unordered_map<std::string, brls::BrlsKeyboardModifiers> modifierMap = {
    {"shift", brls::BRLS_KBD_MODIFIER_SHIFT},
    {"ctrl", brls::BRLS_KBD_MODIFIER_CTRL},
    {"alt", brls::BRLS_KBD_MODIFIER_ALT},
    {"meta", brls::BRLS_KBD_MODIFIER_META}
};

static std::unordered_map<std::string, brls::BrlsKeyboardScancode> functionMap = {
    {"f1", brls::BRLS_KBD_KEY_F1},
    {"f2", brls::BRLS_KBD_KEY_F2},
    {"f3", brls::BRLS_KBD_KEY_F3},
    {"f4", brls::BRLS_KBD_KEY_F4},
    {"f5", brls::BRLS_KBD_KEY_F5},
    {"f6", brls::BRLS_KBD_KEY_F6},
    {"f7", brls::BRLS_KBD_KEY_F7},
    {"f8", brls::BRLS_KBD_KEY_F8},
    {"f9", brls::BRLS_KBD_KEY_F9},
    {"f10", brls::BRLS_KBD_KEY_F10},
    {"f11", brls::BRLS_KBD_KEY_F11},
    {"f12", brls::BRLS_KBD_KEY_F12},
    {"f13", brls::BRLS_KBD_KEY_F13},
    {"f14", brls::BRLS_KBD_KEY_F14},
    {"f15", brls::BRLS_KBD_KEY_F15},
    {"f16", brls::BRLS_KBD_KEY_F16},
    {"f17", brls::BRLS_KBD_KEY_F17},
    {"f18", brls::BRLS_KBD_KEY_F18},
    {"f19", brls::BRLS_KBD_KEY_F19},
    {"f20", brls::BRLS_KBD_KEY_F20},
    {"f21", brls::BRLS_KBD_KEY_F21},
    {"f22", brls::BRLS_KBD_KEY_F22},
    {"f23", brls::BRLS_KBD_KEY_F23},
    {"f24", brls::BRLS_KBD_KEY_F24},
    {"tab", brls::BRLS_KBD_KEY_TAB},
    {"backspace", brls::BRLS_KBD_KEY_BACKSPACE},
    {"insert", brls::BRLS_KBD_KEY_INSERT},
    {"delete", brls::BRLS_KBD_KEY_DELETE},
    {"pgup", brls::BRLS_KBD_KEY_PAGE_UP},
    {"pgdn", brls::BRLS_KBD_KEY_PAGE_DOWN},
    {"home", brls::BRLS_KBD_KEY_HOME},
    {"end", brls::BRLS_KBD_KEY_END},
    {"up", brls::BRLS_KBD_KEY_UP},
    {"down", brls::BRLS_KBD_KEY_DOWN},
    {"left", brls::BRLS_KBD_KEY_LEFT},
    {"right", brls::BRLS_KBD_KEY_RIGHT},
    {"pause", brls::BRLS_KBD_KEY_PAUSE},
    {"menu", brls::BRLS_KBD_KEY_MENU},
    {"space", brls::BRLS_KBD_KEY_SPACE},
    /* support literal names of some printable keys */
    {"apostrophe", brls::BRLS_KBD_KEY_APOSTROPHE}, /* ' */
    {"comma", brls::BRLS_KBD_KEY_COMMA}, /* , */
    {"minus", brls::BRLS_KBD_KEY_MINUS}, /* - */
    {"period", brls::BRLS_KBD_KEY_PERIOD}, /* . */
    {"slash", brls::BRLS_KBD_KEY_SLASH}, /* / */
    {"backslash", brls::BRLS_KBD_KEY_BACKSLASH}, /* \ */
    {"semicolon", brls::BRLS_KBD_KEY_SEMICOLON}, /* ; */
    {"equal", brls::BRLS_KBD_KEY_EQUAL}, /* = */
    {"grave", brls::BRLS_KBD_KEY_GRAVE_ACCENT}, /* ` */
    {"left_bracket", brls::BRLS_KBD_KEY_LEFT_BRACKET}, /* [ */
    {"right_bracket", brls::BRLS_KBD_KEY_RIGHT_BRACKET}, /* ] */
    {"'", brls::BRLS_KBD_KEY_APOSTROPHE},
    {",", brls::BRLS_KBD_KEY_COMMA},
    {".", brls::BRLS_KBD_KEY_PERIOD},
    {"/", brls::BRLS_KBD_KEY_SLASH},
    {"\\", brls::BRLS_KBD_KEY_BACKSLASH},
    {";", brls::BRLS_KBD_KEY_SEMICOLON},
    {"=", brls::BRLS_KBD_KEY_EQUAL},
    {"`", brls::BRLS_KBD_KEY_GRAVE_ACCENT},
    {"[", brls::BRLS_KBD_KEY_LEFT_BRACKET},
    {"]", brls::BRLS_KBD_KEY_RIGHT_BRACKET},
};

static std::string keyDisplayName(brls::BrlsKeyboardScancode code) {
    if (code >= brls::BRLS_KBD_KEY_0 && code <= brls::BRLS_KBD_KEY_9) {
        return std::string(1, static_cast<char>('0' + code - brls::BRLS_KBD_KEY_0));
    }
    if (code >= brls::BRLS_KBD_KEY_A && code <= brls::BRLS_KBD_KEY_Z) {
        return std::string(1, static_cast<char>('A' + code - brls::BRLS_KBD_KEY_A));
    }
    if (code >= brls::BRLS_KBD_KEY_F1 && code <= brls::BRLS_KBD_KEY_F24) {
        return "F" + std::to_string(code - brls::BRLS_KBD_KEY_F1 + 1);
    }

    switch (code) {
        case brls::BRLS_KBD_KEY_TAB:
            return "Tab";
        case brls::BRLS_KBD_KEY_BACKSPACE:
            return "Backspace";
        case brls::BRLS_KBD_KEY_INSERT:
            return "Insert";
        case brls::BRLS_KBD_KEY_DELETE:
            return "Delete";
        case brls::BRLS_KBD_KEY_PAGE_UP:
            return "Page Up";
        case brls::BRLS_KBD_KEY_PAGE_DOWN:
            return "Page Down";
        case brls::BRLS_KBD_KEY_HOME:
            return "Home";
        case brls::BRLS_KBD_KEY_END:
            return "End";
        case brls::BRLS_KBD_KEY_UP:
            return "Up";
        case brls::BRLS_KBD_KEY_DOWN:
            return "Down";
        case brls::BRLS_KBD_KEY_LEFT:
            return "Left";
        case brls::BRLS_KBD_KEY_RIGHT:
            return "Right";
        case brls::BRLS_KBD_KEY_PAUSE:
            return "Pause";
        case brls::BRLS_KBD_KEY_MENU:
            return "Menu";
        case brls::BRLS_KBD_KEY_SPACE:
            return "Space";
        case brls::BRLS_KBD_KEY_APOSTROPHE:
            return "'";
        case brls::BRLS_KBD_KEY_COMMA:
            return ",";
        case brls::BRLS_KBD_KEY_MINUS:
            return "-";
        case brls::BRLS_KBD_KEY_PERIOD:
            return ".";
        case brls::BRLS_KBD_KEY_SLASH:
            return "/";
        case brls::BRLS_KBD_KEY_BACKSLASH:
            return "\\";
        case brls::BRLS_KBD_KEY_SEMICOLON:
            return ";";
        case brls::BRLS_KBD_KEY_EQUAL:
            return "=";
        case brls::BRLS_KBD_KEY_GRAVE_ACCENT:
            return "`";
        case brls::BRLS_KBD_KEY_LEFT_BRACKET:
            return "[";
        case brls::BRLS_KBD_KEY_RIGHT_BRACKET:
            return "]";
        default:
            return {};
    }
}

static void appendDisplayPart(std::string& display, const std::string& part) {
    if (!display.empty()) display += "+";
    display += part;
}

static std::string formatKeyboardBinding(const brls::BrlsKeyCombination& key) {
    std::string display;
    if (key.mod & brls::BRLS_KBD_MODIFIER_CTRL) appendDisplayPart(display, "Ctrl");
    if (key.mod & brls::BRLS_KBD_MODIFIER_ALT) appendDisplayPart(display, "Alt");
    if (key.mod & brls::BRLS_KBD_MODIFIER_SHIFT) appendDisplayPart(display, "Shift");
    if (key.mod & brls::BRLS_KBD_MODIFIER_META) appendDisplayPart(display, "Meta");

    const std::string keyName = keyDisplayName(key.code);
    if (keyName.empty()) return {};
    appendDisplayPart(display, keyName);
    return display;
}

static void appendConfigPart(std::string& config, const std::string& part) {
    if (!config.empty()) config += "-";
    config += part;
}

static std::string keyConfigName(const brls::BrlsKeyCombination& key) {
    std::string keyName = keyDisplayName(key.code);
    if (keyName == "Page Up") return "pgup";
    if (keyName == "Page Down") return "pgdn";
    return pystring::lower(keyName);
}

static brls::BrlsKeyCombination currentShortcut(ShortcutAction action) {
    switch (action) {
        case ShortcutAction::Refresh:
            return ShortcutHelper::getRefresh();
        case ShortcutAction::Search:
            return ShortcutHelper::getSearch();
        case ShortcutAction::Last:
            return ShortcutHelper::getLast();
        case ShortcutAction::Next:
            return ShortcutHelper::getNext();
        case ShortcutAction::LastSub:
            return ShortcutHelper::getLastSub();
        case ShortcutAction::NextSub:
            return ShortcutHelper::getNextSub();
        case ShortcutAction::VolumeUp:
            return ShortcutHelper::getVolumeUp();
        case ShortcutAction::VolumeDown:
            return ShortcutHelper::getVolumeDown();
        case ShortcutAction::VideoProfile:
            return ShortcutHelper::getVideoProfile();
        case ShortcutAction::Danmaku:
            return ShortcutHelper::getDanmaku();
        case ShortcutAction::Playlist:
            return ShortcutHelper::getPlaylist();
        case ShortcutAction::Forward:
            return ShortcutHelper::getForward();
        case ShortcutAction::Rewind:
            return ShortcutHelper::getRewind();
        case ShortcutAction::Setting:
            return ShortcutHelper::getSetting();
        case ShortcutAction::VideoQuality:
            return ShortcutHelper::getVideoQuality();
        case ShortcutAction::VideoSpeed:
            return ShortcutHelper::getVideoSpeed();
        case ShortcutAction::VideoSpeedUp:
            return ShortcutHelper::getVideoSpeedUp();
        case ShortcutAction::VideoOsd:
            return ShortcutHelper::getVideoOsd();
        case ShortcutAction::VideoPause:
            return ShortcutHelper::getVideoPause();
    }
    return {brls::BRLS_KBD_KEY_UNKNOWN};
}

static std::vector<ShortcutHelper::ShortcutRegistration*>& dynamicRegistrations(ShortcutAction action) {
    static std::unordered_map<ShortcutAction, std::vector<ShortcutHelper::ShortcutRegistration*>> registrations;
    return registrations[action];
}

static void refreshDynamicRegistrations(ShortcutAction action) {
    for (auto* registration : dynamicRegistrations(action)) registration->refresh();
}

ShortcutHelper::ShortcutRegistration::ShortcutRegistration(ShortcutAction action, brls::View* view,
                                                          brls::ActionListener listener, bool allowRepeating)
    : action(action), view(view), listener(std::move(listener)), allowRepeating(allowRepeating) {
    dynamicRegistrations(action).push_back(this);
    this->refresh();
}

ShortcutHelper::ShortcutRegistration::~ShortcutRegistration() {
    if (this->view && this->actionIdentifier != -1) this->view->unregisterAction(this->actionIdentifier);

    auto& registrations = dynamicRegistrations(this->action);
    registrations.erase(std::remove(registrations.begin(), registrations.end(), this), registrations.end());
}

void ShortcutHelper::ShortcutRegistration::refresh() {
    if (!this->view) return;
    if (this->actionIdentifier != -1) this->view->unregisterAction(this->actionIdentifier);

    const auto key = currentShortcut(this->action);
    if (key.code == brls::BRLS_KBD_KEY_UNKNOWN) {
        this->actionIdentifier = -1;
        return;
    }

    this->actionIdentifier = this->view->registerAction(key, this->listener, this->allowRepeating);
}

std::unique_ptr<ShortcutHelper::ShortcutRegistration> ShortcutHelper::registerAction(
    ShortcutAction action, brls::View* view, brls::ActionListener listener, bool allowRepeating) {
    return std::make_unique<ShortcutRegistration>(action, view, std::move(listener), allowRepeating);
}

brls::BrlsKeyCombination ShortcutHelper::parseKey(const std::string& config) {
    std::vector<std::string> keys;
    if (pystring::endswith(config, "-")) {
        const std::string cfg = config.substr(0, config.size() - 1) + "minus";
        keys = pystring::split(cfg, "-");
    } else {
        keys = pystring::split(config, "-");
    }
    brls::BrlsKeyCombination res = {brls::BRLS_KBD_KEY_UNKNOWN};
    for (auto& key : keys) {
        key = pystring::strip(key);
        key = pystring::lower(key);
        if (key.empty()) continue;

        if (modifierMap.count(key) > 0) {
            // If it's a modifier, we just set it in the result
            res.mod |= modifierMap[key];
            continue;
        }

        if (functionMap.count(key) > 0) {
            // If it's a function key, we just set it in the result
            res.code = functionMap[key];
            break;
        }

        if (key.length() == 1) {
            const auto keyChar = key[0];
            if (keyChar >= '0' && keyChar <= '9') {
                res.code = static_cast<brls::BrlsKeyboardScancode>(keyChar - '0' + brls::BRLS_KBD_KEY_0);
                break;
            }
            if (keyChar >= 'a' && keyChar <= 'z') {
                res.code = static_cast<brls::BrlsKeyboardScancode>(keyChar - 'a' + brls::BRLS_KBD_KEY_A);
                break;
            }

            brls::Logger::error("Invalid key configuration: {}", config);
            return {brls::BRLS_KBD_KEY_UNKNOWN};
        }

        brls::Logger::error("Invalid key configuration: {}", config);
        return {brls::BRLS_KBD_KEY_UNKNOWN};
    }

    return res;
}

std::string ShortcutBinding::format() const { return ShortcutHelper::formatBinding(*this); }

ShortcutBinding ShortcutHelper::parseBinding(const std::string& config) {
    ShortcutBinding binding;
    binding.key = parseKey(config);
    if (binding.key.code == brls::BRLS_KBD_KEY_UNKNOWN) return binding;

    binding.device = ShortcutDevice::Keyboard;
    binding.display = formatKeyboardBinding(binding.key);
    return binding;
}

std::string ShortcutHelper::gamepadButtonDisplayName(int button) {
    if (button == brls::ControllerButton::BUTTON_A) return "A";
    if (button == brls::ControllerButton::BUTTON_B) return "B";
    if (button == brls::ControllerButton::BUTTON_X) return "X";
    if (button == brls::ControllerButton::BUTTON_Y) return "Y";
    if (button == brls::ControllerButton::BUTTON_LB) return "LB";
    if (button == brls::ControllerButton::BUTTON_RB) return "RB";
    if (button == brls::ControllerButton::BUTTON_LT) return "LT";
    if (button == brls::ControllerButton::BUTTON_RT) return "RT";
    if (button == brls::ControllerButton::BUTTON_START) return "Start";
    if (button == brls::ControllerButton::BUTTON_BACK) return "Back";
    if (button == brls::ControllerButton::BUTTON_LEFT || button == brls::ControllerButton::BUTTON_NAV_LEFT) return "Left";
    if (button == brls::ControllerButton::BUTTON_RIGHT || button == brls::ControllerButton::BUTTON_NAV_RIGHT) return "Right";
    if (button == brls::ControllerButton::BUTTON_NAV_UP) return "Up";
    if (button == brls::ControllerButton::BUTTON_NAV_DOWN) return "Down";
    return {};
}

ShortcutBinding ShortcutHelper::gamepadBinding(int button) {
    ShortcutBinding binding;
    binding.display = gamepadButtonDisplayName(button);
    if (binding.display.empty()) return binding;

    binding.device = ShortcutDevice::Gamepad;
    binding.nativeCode = button;
    return binding;
}

std::string ShortcutHelper::formatBinding(const ShortcutBinding& binding) {
    if (binding.device == ShortcutDevice::Keyboard) {
        const std::string display = formatKeyboardBinding(binding.key);
        if (!display.empty()) return display;
    }
    return binding.display;
}

std::string ShortcutHelper::bindingConfigKey(const ShortcutBinding& binding) {
    if (binding.device != ShortcutDevice::Keyboard) return {};

    std::string config;
    if (binding.key.mod & brls::BRLS_KBD_MODIFIER_CTRL) appendConfigPart(config, "ctrl");
    if (binding.key.mod & brls::BRLS_KBD_MODIFIER_ALT) appendConfigPart(config, "alt");
    if (binding.key.mod & brls::BRLS_KBD_MODIFIER_SHIFT) appendConfigPart(config, "shift");
    if (binding.key.mod & brls::BRLS_KBD_MODIFIER_META) appendConfigPart(config, "meta");

    const std::string keyName = keyConfigName(binding.key);
    if (keyName.empty()) return {};
    appendConfigPart(config, keyName);
    return config;
}

brls::BrlsKeyCombination ShortcutHelper::toBrlsKeyCombination(const ShortcutBinding& binding) {
    if (binding.device == ShortcutDevice::Keyboard) return binding.key;
    return {brls::BRLS_KBD_KEY_UNKNOWN};
}

bool ShortcutHelper::applyBinding(ShortcutAction action, const ShortcutBinding& binding) {
    const std::string config = bindingConfigKey(binding);
    if (config.empty()) return false;

    switch (action) {
        case ShortcutAction::Refresh:
            setRefresh(config);
            break;
        case ShortcutAction::Search:
            setSearch(config);
            break;
        case ShortcutAction::Last:
            setLast(config);
            break;
        case ShortcutAction::Next:
            setNext(config);
            break;
        case ShortcutAction::LastSub:
            setLastSub(config);
            break;
        case ShortcutAction::NextSub:
            setNextSub(config);
            break;
        case ShortcutAction::VolumeUp:
            setVolumeUp(config);
            break;
        case ShortcutAction::VolumeDown:
            setVolumeDown(config);
            break;
        case ShortcutAction::VideoProfile:
            setVideoProfile(config);
            break;
        case ShortcutAction::Danmaku:
            setDanmaku(config);
            break;
        case ShortcutAction::Playlist:
            setPlaylist(config);
            break;
        case ShortcutAction::Forward:
            setForward(config);
            break;
        case ShortcutAction::Rewind:
            setRewind(config);
            break;
        case ShortcutAction::Setting:
            setSetting(config);
            break;
        case ShortcutAction::VideoQuality:
            setVideoQuality(config);
            break;
        case ShortcutAction::VideoSpeed:
            setVideoSpeed(config);
            break;
        case ShortcutAction::VideoSpeedUp:
            setVideoSpeedUp(config);
            break;
        case ShortcutAction::VideoOsd:
            setVideoOsd(config);
            break;
        case ShortcutAction::VideoPause:
            setVideoPause(config);
            break;
    }
    refreshDynamicRegistrations(action);
    return true;
}
