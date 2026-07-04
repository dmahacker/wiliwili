#include "utils/shortcut_capture_helper.hpp"
#include "utils/shortcut_helper.hpp"

#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace {
constexpr int GLFW_KEY_UNKNOWN = -1;
constexpr int GLFW_PRESS = 1;
constexpr int SDL_SCANCODE_AC_BACK = 270;
constexpr int APPCOMMAND_BROWSER_BACKWARD = 1;
constexpr int APPCOMMAND_BROWSER_HOME = 7;
constexpr int RAW_HID_CONSUMER_CF = (0x02 << 16) | 0x00CF;

bool expect(bool condition, const std::string& message) {
    if (condition) return true;
    std::cerr << "FAIL: " << message << '\n';
    return false;
}
}

namespace brls {
std::vector<std::function<void()>> pendingSyncTasks;

void Application::addToWatchedKeys(const BrlsKeyCombination) {}

void sync(const std::function<void()>& func) { pendingSyncTasks.push_back(func); }

void drainSyncTasks() {
    auto tasks = pendingSyncTasks;
    pendingSyncTasks.clear();
    for (const auto& task : tasks) task();
}
}

brls::BrlsKeyCombination ShortcutHelper::parseKey(const std::string& config) {
    if (config == "ctrl-r") return brls::BrlsKeyCombination{brls::BRLS_KBD_KEY_R, brls::BRLS_KBD_MODIFIER_CTRL};
    if (config == "ctrl-f") return brls::BrlsKeyCombination{brls::BRLS_KBD_KEY_F, brls::BRLS_KBD_MODIFIER_CTRL};
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

    bool callbackCalled = false;
    ShortcutCaptureHelper::setNativeCaptureCallback([&](const ShortcutBinding& binding) {
        callbackCalled = binding.device == ShortcutDevice::WindowsAppCommand &&
                         binding.nativeCode == APPCOMMAND_BROWSER_BACKWARD;
    });
    ShortcutCaptureHelper::startCapture();
    ShortcutCaptureHelper::publishNativeCapture(appCommandBack);
    ok &= expect(!callbackCalled, "Native capture callback should be queued for main thread");
    brls::drainSyncTasks();
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

    dispatchedAction = ShortcutAction::Confirm;
    runtimeDispatched = false;
    ShortcutCaptureHelper::setNativeShortcut(ShortcutAction::Back, appCommandHome);
    ok &= expect(ShortcutCaptureHelper::publishNativeShortcut(appCommandHome),
                 "Browser Home native shortcut should be consumed at runtime");
    ok &= expect(runtimeDispatched, "Browser Home native shortcut should dispatch an action at runtime");
    ok &= expect(dispatchedAction == ShortcutAction::Back, "Browser Home native shortcut should dispatch Back action");
    ShortcutCaptureHelper::clearNativeShortcuts();

    ok &= expect(ShortcutBindingHelper::applyBinding(ShortcutAction::Refresh, appCommandHome),
                 "Native Browser Home binding should apply to Refresh");
    ok &= expect(ShortcutHelper::getRefresh().code == brls::BRLS_KBD_KEY_R,
                 "Native Refresh binding should keep Refresh keyboard fallback");
    ok &= expect(ShortcutHelper::getRefresh().mod == brls::BRLS_KBD_MODIFIER_CTRL,
                 "Native Refresh binding should keep Ctrl modifier fallback");

    const unsigned char rawHomePress[] = {0x02, 0xCF, 0x00};
    const unsigned char rawHomeRelease[] = {0x02, 0x00, 0x00};
    const unsigned char rawShortReport[] = {0x02, 0xCF};
    const auto rawHidKey = ShortcutCaptureHelper::mapWindowsRawInputHidReport(rawHomePress, sizeof(rawHomePress));
    ok &= expect(rawHidKey.device == ShortcutDevice::WindowsRawHid,
                 "Raw HID 0x02CF00 report should map to raw HID binding");
    ok &= expect(rawHidKey.nativeCode == RAW_HID_CONSUMER_CF,
                 "Raw HID 0x02CF00 report should preserve report id and usage");
    ok &= expect(rawHidKey.display == "HID Consumer 0x00CF", "Raw HID 0x02CF00 report should have HID display");
    ok &= expect(ShortcutBindingHelper::bindingConfigKey(rawHidKey) == "rawhid-2-207",
                 "Raw HID 0x02CF00 report should serialize to stable config");

    const auto parsedRawHid = ShortcutBindingHelper::parseBinding("rawhid-2-207");
    ok &= expect(parsedRawHid.device == ShortcutDevice::WindowsRawHid,
                 "Raw HID config should parse as Windows Raw HID binding");
    ok &= expect(parsedRawHid.nativeCode == RAW_HID_CONSUMER_CF,
                 "Raw HID config should preserve report id and usage");
    ok &= expect(parsedRawHid.display == "HID Consumer 0x00CF", "Raw HID config should restore HID display");

    ok &= expect(ShortcutBindingHelper::applyBinding(ShortcutAction::Search, rawHidKey),
                 "Raw HID binding should apply to Search");
    ok &= expect(ShortcutHelper::getSearch().code == brls::BRLS_KBD_KEY_F,
                 "Raw HID Search binding should keep Search keyboard fallback");
    ok &= expect(ShortcutHelper::getSearch().mod == brls::BRLS_KBD_MODIFIER_CTRL,
                 "Raw HID Search binding should keep Ctrl modifier fallback");

    const auto rawRelease = ShortcutCaptureHelper::mapWindowsRawInputHidReport(rawHomeRelease, sizeof(rawHomeRelease));
    ok &= expect(rawRelease.device == ShortcutDevice::Unsupported, "Raw HID Browser Home release should be ignored");
    const auto rawMalformed = ShortcutCaptureHelper::mapWindowsRawInputHidReport(rawShortReport, sizeof(rawShortReport));
    ok &= expect(rawMalformed.device == ShortcutDevice::Unsupported, "Short Raw HID reports should be ignored");

    callbackCalled = false;
    ShortcutCaptureHelper::setNativeCaptureCallback([&](const ShortcutBinding& binding) {
        callbackCalled = binding.device == ShortcutDevice::WindowsRawHid && binding.nativeCode == RAW_HID_CONSUMER_CF;
    });
    ShortcutCaptureHelper::startCapture();
    ShortcutCaptureHelper::publishNativeCapture(rawHidKey);
    ok &= expect(!callbackCalled, "Raw HID capture callback should be queued for main thread");
    brls::drainSyncTasks();
    ShortcutCaptureHelper::stopCapture();
    ok &= expect(callbackCalled, "Native capture callback should receive raw HID binding while capturing");

    dispatchedAction = ShortcutAction::Confirm;
    runtimeDispatched = false;
    ShortcutCaptureHelper::setNativeShortcut(ShortcutAction::Back, rawHidKey);
    ok &= expect(ShortcutCaptureHelper::publishNativeShortcut(rawHidKey),
                 "Raw HID native shortcut should be consumed at runtime");
    ok &= expect(runtimeDispatched, "Raw HID native shortcut should dispatch an action at runtime");
    ok &= expect(dispatchedAction == ShortcutAction::Back, "Raw HID native shortcut should dispatch Back action");
    ShortcutCaptureHelper::clearNativeShortcuts();

    return ok ? 0 : 1;
}
