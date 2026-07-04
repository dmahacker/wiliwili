#include "utils/shortcut_capture_helper.hpp"
#include "utils/shortcut_helper.hpp"

#include <iostream>
#include <string>

namespace {
constexpr int GLFW_KEY_UNKNOWN = -1;
constexpr int GLFW_PRESS = 1;
constexpr int SDL_SCANCODE_AC_BACK = 270;
constexpr int APPCOMMAND_BROWSER_BACKWARD = 1;
constexpr int APPCOMMAND_BROWSER_HOME = 7;

bool expect(bool condition, const std::string& message) {
    if (condition) return true;
    std::cerr << "FAIL: " << message << '\n';
    return false;
}
}

brls::BrlsKeyCombination ShortcutHelper::parseKey(const std::string& config) {
    if (config == "escape") return brls::BrlsKeyCombination{brls::BRLS_KBD_KEY_ESCAPE};
    return brls::BrlsKeyCombination{brls::BRLS_KBD_KEY_UNKNOWN};
}

int main() {
    const auto scancodeBack = ShortcutCaptureHelper::mapWindowsKeyboardEvent(
        GLFW_KEY_UNKNOWN, SDL_SCANCODE_AC_BACK, GLFW_PRESS, 0);
    bool ok = true;
    ok &= expect(scancodeBack.device == ShortcutDevice::Keyboard, "AC Back scancode should be recognized");
    ok &= expect(scancodeBack.nativeCode == SDL_SCANCODE_AC_BACK, "AC Back scancode should be preserved");
    ok &= expect(scancodeBack.display == "Browser Back", "AC Back scancode should have browser back display");
    ok &= expect(ShortcutBindingHelper::bindingConfigKey(scancodeBack) == "scancode-270",
                 "AC Back scancode should serialize to stable config");

    const auto parsedScancodeBack = ShortcutBindingHelper::parseBinding("scancode-270");
    ok &= expect(parsedScancodeBack.device == ShortcutDevice::Keyboard, "Scancode config should parse as keyboard binding");
    ok &= expect(parsedScancodeBack.nativeCode == SDL_SCANCODE_AC_BACK, "Scancode config should preserve native code");
    ok &= expect(parsedScancodeBack.display == "Browser Back", "Scancode config should restore browser back display");

    const auto appCommandBack = ShortcutCaptureHelper::mapWindowsAppCommandEvent(APPCOMMAND_BROWSER_BACKWARD);
    ok &= expect(appCommandBack.device == ShortcutDevice::WindowsAppCommand, "Browser Back app command should be recognized");
    ok &= expect(appCommandBack.nativeCode == APPCOMMAND_BROWSER_BACKWARD, "Browser Back app command should be preserved");
    ok &= expect(appCommandBack.display == "Browser Back", "Browser Back app command should have browser back display");
    ok &= expect(ShortcutBindingHelper::bindingConfigKey(appCommandBack) == "appcommand-1",
                 "Browser Back app command should serialize to stable config");

    const auto parsedAppCommandBack = ShortcutBindingHelper::parseBinding("appcommand-1");
    ok &= expect(parsedAppCommandBack.device == ShortcutDevice::WindowsAppCommand,
                 "App command config should parse as Windows app command binding");
    ok &= expect(parsedAppCommandBack.nativeCode == APPCOMMAND_BROWSER_BACKWARD,
                 "App command config should preserve native command");
    ok &= expect(parsedAppCommandBack.display == "Browser Back", "App command config should restore browser back display");

    bool callbackCalled = false;
    const auto appCommandHome = ShortcutCaptureHelper::mapWindowsAppCommandEvent(APPCOMMAND_BROWSER_HOME);
    ok &= expect(appCommandHome.device == ShortcutDevice::WindowsAppCommand, "Browser Home app command should be recognized");
    ok &= expect(appCommandHome.nativeCode == APPCOMMAND_BROWSER_HOME, "Browser Home app command should be preserved");
    ok &= expect(appCommandHome.display == "Browser Home", "Browser Home app command should have browser home display");
    ok &= expect(ShortcutBindingHelper::bindingConfigKey(appCommandHome) == "appcommand-7",
                 "Browser Home app command should serialize to stable config");

    const auto parsedAppCommandHome = ShortcutBindingHelper::parseBinding("appcommand-7");
    ok &= expect(parsedAppCommandHome.device == ShortcutDevice::WindowsAppCommand,
                 "Browser Home config should parse as Windows app command binding");
    ok &= expect(parsedAppCommandHome.nativeCode == APPCOMMAND_BROWSER_HOME,
                 "Browser Home config should preserve native command");
    ok &= expect(parsedAppCommandHome.display == "Browser Home", "Browser Home config should restore browser home display");

    ShortcutCaptureHelper::setNativeCaptureCallback([&](const ShortcutBinding& binding) {
        callbackCalled = binding.device == ShortcutDevice::WindowsAppCommand &&
                         binding.nativeCode == APPCOMMAND_BROWSER_BACKWARD;
    });
    ShortcutCaptureHelper::startCapture();
    ShortcutCaptureHelper::publishNativeCapture(appCommandBack);
    ShortcutCaptureHelper::stopCapture();
    ok &= expect(callbackCalled, "Native capture callback should receive browser back binding while capturing");

    ShortcutAction dispatchedAction = ShortcutAction::Confirm;
    bool runtimeDispatched = false;
    ShortcutCaptureHelper::setNativeDispatchCallback([&](ShortcutAction action) {
        dispatchedAction = action;
        runtimeDispatched = true;
        return true;
    });
    ShortcutCaptureHelper::setNativeShortcut(ShortcutAction::Back, appCommandBack);
    ok &= expect(ShortcutCaptureHelper::publishNativeShortcut(appCommandBack),
                 "Browser Back native shortcut should be consumed at runtime");
    ok &= expect(runtimeDispatched, "Browser Back native shortcut should dispatch an action at runtime");
    ok &= expect(dispatchedAction == ShortcutAction::Back, "Browser Back native shortcut should dispatch Back action");
    ShortcutCaptureHelper::clearNativeShortcuts();

    return ok ? 0 : 1;
    dispatchedAction = ShortcutAction::Confirm;
    runtimeDispatched = false;
    ShortcutCaptureHelper::setNativeShortcut(ShortcutAction::Back, appCommandHome);
    ok &= expect(ShortcutCaptureHelper::publishNativeShortcut(appCommandHome),
                 "Browser Home native shortcut should be consumed at runtime");
    ok &= expect(runtimeDispatched, "Browser Home native shortcut should dispatch an action at runtime");
    ok &= expect(dispatchedAction == ShortcutAction::Back, "Browser Home native shortcut should dispatch Back action");
    ShortcutCaptureHelper::clearNativeShortcuts();

}
