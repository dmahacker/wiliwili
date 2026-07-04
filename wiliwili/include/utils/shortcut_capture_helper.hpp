#pragma once

#include <cstddef>
#include <functional>

#include "utils/shortcut_binding.hpp"

enum class ShortcutCaptureStatus {
    Idle,
    Capturing,
};

class ShortcutCaptureHelper {
public:
    static void startCapture();
    static void stopCapture();
    static bool isCapturing();
    static void setNativeCaptureCallback(std::function<void(const ShortcutBinding&)> callback);
    static void publishNativeCapture(const ShortcutBinding& binding);
    static void setNativeDispatchCallback(std::function<bool(ShortcutAction)> callback);
    static void setNativeShortcut(ShortcutAction action, const ShortcutBinding& binding);
    static bool publishNativeShortcut(const ShortcutBinding& binding);
    static void clearNativeShortcuts();

    static ShortcutBinding mapWindowsKeyboardEvent(int key, int scancode, int action, int mods);
    static ShortcutBinding mapWindowsAppCommandEvent(int command);
    static ShortcutBinding mapWindowsRawInputHidReport(const unsigned char* report, std::size_t size);

private:
    static void startNativeCapture();
    static void stopNativeCapture();
    static bool hasNativeShortcuts();
};
