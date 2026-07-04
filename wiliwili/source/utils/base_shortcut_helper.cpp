//
// Base navigation shortcuts (Confirm / Back / Navigate*).
//

#include "utils/base_shortcut_helper.hpp"

#include "utils/config_helper.hpp"
#include "utils/shortcut_binding.hpp"
#include "utils/shortcut_capture_helper.hpp"

static bool shortcutMatchesKeyState(const brls::BrlsKeyCombination& shortcut, const brls::KeyState& state) {
    return shortcut.code != brls::BRLS_KBD_KEY_UNKNOWN && shortcut.code == state.key && shortcut.mod == state.mods;
}

static bool isNativeControllerKeyState(const brls::KeyState& state) {
    if (state.mods != 0) return false;
    return state.key == brls::BRLS_KBD_KEY_ENTER || state.key == brls::BRLS_KBD_KEY_ESCAPE ||
           state.key == brls::BRLS_KBD_KEY_UP || state.key == brls::BRLS_KBD_KEY_DOWN ||
           state.key == brls::BRLS_KBD_KEY_LEFT || state.key == brls::BRLS_KBD_KEY_RIGHT;
}

static bool dispatchButtonOnce(const brls::BrlsKeyCombination& shortcut, const brls::KeyState& state, bool& shortcutHeld,
                               brls::ControllerButton button) {
    if (state.key == shortcut.code && !state.pressed) {
        shortcutHeld = false;
        return false;
    }
    if (brls::Application::isInputBlocks()) return false;
    if (!shortcutMatchesKeyState(shortcut, state) || isNativeControllerKeyState(state)) return false;
    if (shortcutHeld) return true;

    shortcutHeld = true;
    brls::Application::onControllerButtonPressed(button, false);
    return true;
}

static bool isNativeShortcutBinding(const ShortcutBinding& binding) {
    if (binding.device == ShortcutDevice::WindowsAppCommand || binding.device == ShortcutDevice::WindowsRawHid) return true;
    return binding.device == ShortcutDevice::Keyboard && binding.key.code == brls::BRLS_KBD_KEY_UNKNOWN &&
           binding.nativeCode > 0;
}

static std::string nativeSafeConfig(const std::string& config, const std::string& defaultConfig) {
    const ShortcutBinding binding = ShortcutBindingHelper::parseBinding(config);
    return isNativeShortcutBinding(binding) ? defaultConfig : config;
}

static void loadBaseShortcut(ShortcutAction action, const std::string& config) {
    ShortcutCaptureHelper::setNativeShortcut(action, ShortcutBindingHelper::parseBinding(config));
}

static bool dispatchNativeBaseShortcut(ShortcutAction action) {
    auto dispatchKeyboardShortcut = [](const brls::BrlsKeyCombination& shortcut) {
        if (shortcut.code == brls::BRLS_KBD_KEY_UNKNOWN) return false;
        brls::Application::onKeyboardPressed(shortcut, false);
        return true;
    };

    switch (action) {
        case ShortcutAction::Confirm:
            brls::Application::onControllerButtonPressed(brls::BUTTON_A, false);
            return true;
        case ShortcutAction::Back:
            brls::Application::onControllerButtonPressed(brls::BUTTON_B, false);
            return true;
        case ShortcutAction::NavigateUp:
            brls::Application::onControllerButtonPressed(brls::BUTTON_NAV_UP, false);
            return true;
        case ShortcutAction::NavigateDown:
            brls::Application::onControllerButtonPressed(brls::BUTTON_NAV_DOWN, false);
            return true;
        case ShortcutAction::NavigateLeft:
            brls::Application::onControllerButtonPressed(brls::BUTTON_NAV_LEFT, false);
            return true;
        case ShortcutAction::NavigateRight:
            brls::Application::onControllerButtonPressed(brls::BUTTON_NAV_RIGHT, false);
            return true;
        case ShortcutAction::Refresh:
            return dispatchKeyboardShortcut(ShortcutHelper::getRefresh());
        case ShortcutAction::Search:
            return dispatchKeyboardShortcut(ShortcutHelper::getSearch());
        case ShortcutAction::Last:
            return dispatchKeyboardShortcut(ShortcutHelper::getLast());
        case ShortcutAction::Next:
            return dispatchKeyboardShortcut(ShortcutHelper::getNext());
        case ShortcutAction::LastSub:
            return dispatchKeyboardShortcut(ShortcutHelper::getLastSub());
        case ShortcutAction::NextSub:
            return dispatchKeyboardShortcut(ShortcutHelper::getNextSub());
        case ShortcutAction::VolumeUp:
            return dispatchKeyboardShortcut(ShortcutHelper::getVolumeUp());
        case ShortcutAction::VolumeDown:
            return dispatchKeyboardShortcut(ShortcutHelper::getVolumeDown());
        case ShortcutAction::VideoProfile:
            return dispatchKeyboardShortcut(ShortcutHelper::getVideoProfile());
        case ShortcutAction::Danmaku:
            return dispatchKeyboardShortcut(ShortcutHelper::getDanmaku());
        case ShortcutAction::Playlist:
            return dispatchKeyboardShortcut(ShortcutHelper::getPlaylist());
        case ShortcutAction::Forward:
            return dispatchKeyboardShortcut(ShortcutHelper::getForward());
        case ShortcutAction::Rewind:
            return dispatchKeyboardShortcut(ShortcutHelper::getRewind());
        case ShortcutAction::Setting:
            return dispatchKeyboardShortcut(ShortcutHelper::getSetting());
        case ShortcutAction::VideoQuality:
            return dispatchKeyboardShortcut(ShortcutHelper::getVideoQuality());
        case ShortcutAction::VideoSpeed:
            return dispatchKeyboardShortcut(ShortcutHelper::getVideoSpeed());
        case ShortcutAction::VideoSpeedUp:
            return dispatchKeyboardShortcut(ShortcutHelper::getVideoSpeedUp());
        case ShortcutAction::VideoOsd:
            return dispatchKeyboardShortcut(ShortcutHelper::getVideoOsd());
        case ShortcutAction::VideoPause:
            return dispatchKeyboardShortcut(ShortcutHelper::getVideoPause());
        default:
            return false;
    }
}

void BaseShortcutHelper::loadFromConfig(ProgramConfig& config) {
    ShortcutCaptureHelper::setNativeDispatchCallback(dispatchNativeBaseShortcut);

    const std::string confirmDefault = "enter";
    const std::string backDefault = "escape";
    const std::string navigateUpDefault = "up";
    const std::string navigateDownDefault = "down";
    const std::string navigateLeftDefault = "left";
    const std::string navigateRightDefault = "right";

    const std::string confirm = config.getSettingItem(SettingItem::SHORTCUT_CONFIRM, confirmDefault);
    const std::string back = config.getSettingItem(SettingItem::SHORTCUT_BACK, backDefault);
    const std::string navigateUp = config.getSettingItem(SettingItem::SHORTCUT_NAVIGATE_UP, navigateUpDefault);
    const std::string navigateDown = config.getSettingItem(SettingItem::SHORTCUT_NAVIGATE_DOWN, navigateDownDefault);
    const std::string navigateLeft = config.getSettingItem(SettingItem::SHORTCUT_NAVIGATE_LEFT, navigateLeftDefault);
    const std::string navigateRight = config.getSettingItem(SettingItem::SHORTCUT_NAVIGATE_RIGHT, navigateRightDefault);

    setConfirm(nativeSafeConfig(confirm, confirmDefault));
    setBack(nativeSafeConfig(back, backDefault));
    setNavigateUp(nativeSafeConfig(navigateUp, navigateUpDefault));
    setNavigateDown(nativeSafeConfig(navigateDown, navigateDownDefault));
    setNavigateLeft(nativeSafeConfig(navigateLeft, navigateLeftDefault));
    setNavigateRight(nativeSafeConfig(navigateRight, navigateRightDefault));

    loadBaseShortcut(ShortcutAction::Confirm, confirm);
    loadBaseShortcut(ShortcutAction::Back, back);
    loadBaseShortcut(ShortcutAction::NavigateUp, navigateUp);
    loadBaseShortcut(ShortcutAction::NavigateDown, navigateDown);
    loadBaseShortcut(ShortcutAction::NavigateLeft, navigateLeft);
    loadBaseShortcut(ShortcutAction::NavigateRight, navigateRight);
}

bool BaseShortcutHelper::dispatch(const brls::KeyState& state) {
    static bool confirmShortcutHeld       = false;
    static bool backShortcutHeld          = false;
    static bool navigateUpShortcutHeld    = false;
    static bool navigateDownShortcutHeld  = false;
    static bool navigateLeftShortcutHeld  = false;
    static bool navigateRightShortcutHeld = false;

    if (dispatchButtonOnce(getConfirm(), state, confirmShortcutHeld, brls::BUTTON_A)) return true;
    if (dispatchButtonOnce(getBack(), state, backShortcutHeld, brls::BUTTON_B)) return true;
    if (dispatchButtonOnce(getNavigateUp(), state, navigateUpShortcutHeld, brls::BUTTON_NAV_UP)) return true;
    if (dispatchButtonOnce(getNavigateDown(), state, navigateDownShortcutHeld, brls::BUTTON_NAV_DOWN)) return true;
    if (dispatchButtonOnce(getNavigateLeft(), state, navigateLeftShortcutHeld, brls::BUTTON_NAV_LEFT)) return true;
    if (dispatchButtonOnce(getNavigateRight(), state, navigateRightShortcutHeld, brls::BUTTON_NAV_RIGHT)) return true;
    return false;
}
