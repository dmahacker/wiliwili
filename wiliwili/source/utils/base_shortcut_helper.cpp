//
// Base navigation shortcuts (Confirm / Back / Navigate*).
//

#include "utils/base_shortcut_helper.hpp"

#include "utils/config_helper.hpp"

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

void BaseShortcutHelper::loadFromConfig(ProgramConfig& config) {
    setConfirm(config.getSettingItem(SettingItem::SHORTCUT_CONFIRM, std::string{"enter"}));
    setBack(config.getSettingItem(SettingItem::SHORTCUT_BACK, std::string{"escape"}));
    setNavigateUp(config.getSettingItem(SettingItem::SHORTCUT_NAVIGATE_UP, std::string{"up"}));
    setNavigateDown(config.getSettingItem(SettingItem::SHORTCUT_NAVIGATE_DOWN, std::string{"down"}));
    setNavigateLeft(config.getSettingItem(SettingItem::SHORTCUT_NAVIGATE_LEFT, std::string{"left"}));
    setNavigateRight(config.getSettingItem(SettingItem::SHORTCUT_NAVIGATE_RIGHT, std::string{"right"}));
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
