#pragma once

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

    static ShortcutBinding mapWindowsKeyboardEvent(int key, int scancode, int action, int mods);
    static ShortcutBinding mapWindowsAppCommandEvent(int command);

private:
    static void startNativeCapture();
    static void stopNativeCapture();
};
