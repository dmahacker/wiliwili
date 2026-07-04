//
// Custom shortcut binding model (branch-owned).
//

#include "utils/shortcut_binding.hpp"

#include <cctype>
#include <limits>

#include <pystring.h>

#include "utils/base_shortcut_helper.hpp"
#include "utils/shortcut_helper.hpp"

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
        case brls::BRLS_KBD_KEY_ENTER:
            return "Enter";
        case brls::BRLS_KBD_KEY_ESCAPE:
            return "Escape";
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

static bool parseNonNegativeInt(const std::string& value, int& result) {
    if (value.empty()) return false;

    int parsed = 0;
    for (char c : value) {
        if (!std::isdigit(static_cast<unsigned char>(c))) return false;
        const int digit = c - '0';
        if (parsed > (std::numeric_limits<int>::max() - digit) / 10) return false;
        parsed = parsed * 10 + digit;
    }

    result = parsed;
    return true;
}

static std::string nativeKeyboardDisplayName(int nativeCode) {
    switch (nativeCode) {
        case 128:
            return "Volume Up";
        case 129:
            return "Volume Down";
        case 258:
            return "Media Next";
        case 259:
            return "Media Previous";
        case 260:
            return "Media Stop";
        case 261:
            return "Media Play/Pause";
        case 262:
            return "Mute";
        case 270:
            return "Browser Back";
        case 271:
            return "Browser Forward";
        default:
            return "Scancode " + std::to_string(nativeCode);
    }
}

static std::string appCommandDisplayName(int command) {
    switch (command) {
        case 1:
            return "Browser Back";
        case 2:
            return "Browser Forward";
        case 8:
            return "Mute";
        case 7:
            return "Browser Home";
        case 9:
            return "Volume Down";
        case 10:
            return "Volume Up";
        case 11:
            return "Media Next";
        case 12:
            return "Media Previous";
        case 13:
            return "Media Stop";
        case 14:
            return "Media Play/Pause";
        case 46:
            return "Media Play";
        case 47:
            return "Media Pause";
        default:
            return "App Command " + std::to_string(command);
    }
}

static ShortcutBinding nativeKeyboardBinding(int nativeCode) {
    ShortcutBinding binding;
    binding.device     = ShortcutDevice::Keyboard;
    binding.nativeCode = nativeCode;
    binding.display    = nativeKeyboardDisplayName(nativeCode);
    return binding;
}

static ShortcutBinding appCommandBinding(int command) {
    ShortcutBinding binding;
    binding.device     = ShortcutDevice::WindowsAppCommand;
    binding.nativeCode = command;
    binding.display    = appCommandDisplayName(command);
    return binding;
}

static ShortcutBinding parseNativeBinding(const std::string& config) {
    int nativeCode = 0;
    if (config.rfind("scancode-", 0) == 0 && parseNonNegativeInt(config.substr(9), nativeCode)) {
        return nativeKeyboardBinding(nativeCode);
    }
    if (config.rfind("appcommand-", 0) == 0 && parseNonNegativeInt(config.substr(11), nativeCode)) {
        return appCommandBinding(nativeCode);
    }
    return {};
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

std::string ShortcutBinding::format() const { return ShortcutBindingHelper::formatBinding(*this); }

ShortcutBinding ShortcutBindingHelper::parseBinding(const std::string& config) {
    ShortcutBinding nativeBinding = parseNativeBinding(config);
    if (nativeBinding.device != ShortcutDevice::Unsupported) return nativeBinding;

    ShortcutBinding binding;
    binding.key = ShortcutHelper::parseKey(config);
    if (binding.key.code == brls::BRLS_KBD_KEY_UNKNOWN) return binding;

    binding.device = ShortcutDevice::Keyboard;
    binding.display = formatKeyboardBinding(binding.key);
    return binding;
}

std::string ShortcutBindingHelper::gamepadButtonDisplayName(int button) {
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
    if (button == brls::ControllerButton::BUTTON_RIGHT || button == brls::ControllerButton::BUTTON_NAV_RIGHT)
        return "Right";
    if (button == brls::ControllerButton::BUTTON_NAV_UP) return "Up";
    if (button == brls::ControllerButton::BUTTON_NAV_DOWN) return "Down";
    return {};
}

ShortcutBinding ShortcutBindingHelper::gamepadBinding(int button) {
    ShortcutBinding binding;
    binding.display = gamepadButtonDisplayName(button);
    if (binding.display.empty()) return binding;

    binding.device = ShortcutDevice::Gamepad;
    binding.nativeCode = button;
    return binding;
}

std::string ShortcutBindingHelper::formatBinding(const ShortcutBinding& binding) {
    if (binding.device == ShortcutDevice::Keyboard) {
        const std::string display = formatKeyboardBinding(binding.key);
        if (!display.empty()) return display;
    }
    return binding.display;
}

std::string ShortcutBindingHelper::bindingConfigKey(const ShortcutBinding& binding) {
    if (binding.device == ShortcutDevice::WindowsAppCommand && binding.nativeCode > 0) {
        return "appcommand-" + std::to_string(binding.nativeCode);
    }
    if (binding.device != ShortcutDevice::Keyboard) return {};
    if (binding.key.code == brls::BRLS_KBD_KEY_UNKNOWN && binding.nativeCode > 0) {
        return "scancode-" + std::to_string(binding.nativeCode);
    }

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

brls::BrlsKeyCombination ShortcutBindingHelper::toBrlsKeyCombination(const ShortcutBinding& binding) {
    if (binding.device == ShortcutDevice::Keyboard) return binding.key;
    return {brls::BRLS_KBD_KEY_UNKNOWN};
}

bool ShortcutBindingHelper::applyBinding(ShortcutAction action, const ShortcutBinding& binding) {
    const std::string config = bindingConfigKey(binding);
    if (config.empty()) return false;

    switch (action) {
        case ShortcutAction::Confirm:
            BaseShortcutHelper::setConfirm(config);
            break;
        case ShortcutAction::Back:
            BaseShortcutHelper::setBack(config);
            break;
        case ShortcutAction::NavigateUp:
            BaseShortcutHelper::setNavigateUp(config);
            break;
        case ShortcutAction::NavigateDown:
            BaseShortcutHelper::setNavigateDown(config);
            break;
        case ShortcutAction::NavigateLeft:
            BaseShortcutHelper::setNavigateLeft(config);
            break;
        case ShortcutAction::NavigateRight:
            BaseShortcutHelper::setNavigateRight(config);
            break;
        case ShortcutAction::Refresh:
            ShortcutHelper::setRefresh(config);
            break;
        case ShortcutAction::Search:
            ShortcutHelper::setSearch(config);
            break;
        case ShortcutAction::Last:
            ShortcutHelper::setLast(config);
            break;
        case ShortcutAction::Next:
            ShortcutHelper::setNext(config);
            break;
        case ShortcutAction::LastSub:
            ShortcutHelper::setLastSub(config);
            break;
        case ShortcutAction::NextSub:
            ShortcutHelper::setNextSub(config);
            break;
        case ShortcutAction::VolumeUp:
            ShortcutHelper::setVolumeUp(config);
            break;
        case ShortcutAction::VolumeDown:
            ShortcutHelper::setVolumeDown(config);
            break;
        case ShortcutAction::VideoProfile:
            ShortcutHelper::setVideoProfile(config);
            break;
        case ShortcutAction::Danmaku:
            ShortcutHelper::setDanmaku(config);
            break;
        case ShortcutAction::Playlist:
            ShortcutHelper::setPlaylist(config);
            break;
        case ShortcutAction::Forward:
            ShortcutHelper::setForward(config);
            break;
        case ShortcutAction::Rewind:
            ShortcutHelper::setRewind(config);
            break;
        case ShortcutAction::Setting:
            ShortcutHelper::setSetting(config);
            break;
        case ShortcutAction::VideoQuality:
            ShortcutHelper::setVideoQuality(config);
            break;
        case ShortcutAction::VideoSpeed:
            ShortcutHelper::setVideoSpeed(config);
            break;
        case ShortcutAction::VideoSpeedUp:
            ShortcutHelper::setVideoSpeedUp(config);
            break;
        case ShortcutAction::VideoOsd:
            ShortcutHelper::setVideoOsd(config);
            break;
        case ShortcutAction::VideoPause:
            ShortcutHelper::setVideoPause(config);
            break;
    }
    return true;
}
