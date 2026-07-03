#pragma once

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

    static ShortcutBinding mapWindowsKeyboardEvent(int key, int scancode, int action, int mods);
    static ShortcutBinding mapWindowsAppCommandEvent(int command);
};
