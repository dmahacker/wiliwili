#include "utils/shortcut_capture_helper.hpp"

#include <string>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace {
constexpr int GLFW_PRESS = 1;
constexpr int GLFW_KEY_UNKNOWN = -1;
constexpr int GLFW_MOD_SHIFT = 0x0001;
constexpr int GLFW_MOD_CONTROL = 0x0002;
constexpr int GLFW_MOD_ALT = 0x0004;
constexpr int GLFW_MOD_SUPER = 0x0008;

constexpr int GLFW_KEY_SPACE = 32;
constexpr int GLFW_KEY_APOSTROPHE = 39;
constexpr int GLFW_KEY_COMMA = 44;
constexpr int GLFW_KEY_MINUS = 45;
constexpr int GLFW_KEY_PERIOD = 46;
constexpr int GLFW_KEY_SLASH = 47;
constexpr int GLFW_KEY_0 = 48;
constexpr int GLFW_KEY_9 = 57;
constexpr int GLFW_KEY_SEMICOLON = 59;
constexpr int GLFW_KEY_EQUAL = 61;
constexpr int GLFW_KEY_A = 65;
constexpr int GLFW_KEY_Z = 90;
constexpr int GLFW_KEY_LEFT_BRACKET = 91;
constexpr int GLFW_KEY_BACKSLASH = 92;
constexpr int GLFW_KEY_RIGHT_BRACKET = 93;
constexpr int GLFW_KEY_GRAVE_ACCENT = 96;
constexpr int GLFW_KEY_TAB = 258;
constexpr int GLFW_KEY_BACKSPACE = 259;
constexpr int GLFW_KEY_INSERT = 260;
constexpr int GLFW_KEY_DELETE = 261;
constexpr int GLFW_KEY_RIGHT = 262;
constexpr int GLFW_KEY_LEFT = 263;
constexpr int GLFW_KEY_DOWN = 264;
constexpr int GLFW_KEY_UP = 265;
constexpr int GLFW_KEY_PAGE_UP = 266;
constexpr int GLFW_KEY_PAGE_DOWN = 267;
constexpr int GLFW_KEY_HOME = 268;
constexpr int GLFW_KEY_END = 269;
constexpr int GLFW_KEY_PAUSE = 284;
constexpr int GLFW_KEY_F1 = 290;
constexpr int GLFW_KEY_F24 = 313;
constexpr int GLFW_KEY_MENU = 348;

constexpr int SDL_SCANCODE_AUDIONEXT = 258;
constexpr int SDL_SCANCODE_AUDIOPREV = 259;
constexpr int SDL_SCANCODE_AUDIOSTOP = 260;
constexpr int SDL_SCANCODE_AUDIOPLAY = 261;
constexpr int SDL_SCANCODE_AUDIOMUTE = 262;
constexpr int SDL_SCANCODE_AC_BACK = 270;
constexpr int SDL_SCANCODE_AC_FORWARD = 271;
constexpr int SDL_SCANCODE_VOLUMEUP = 128;
constexpr int SDL_SCANCODE_VOLUMEDOWN = 129;

// On Windows these names are provided as macros by <windows.h> (winuser.h),
// so only define our own fallbacks when that header isn't in play. The values
// intentionally match the real APPCOMMAND_* constants.
#ifndef _WIN32
constexpr int APPCOMMAND_BROWSER_BACKWARD = 1;
constexpr int APPCOMMAND_BROWSER_FORWARD = 2;
constexpr int APPCOMMAND_VOLUME_MUTE = 8;
constexpr int APPCOMMAND_VOLUME_DOWN = 9;
constexpr int APPCOMMAND_VOLUME_UP = 10;
constexpr int APPCOMMAND_MEDIA_NEXTTRACK = 11;
constexpr int APPCOMMAND_MEDIA_PREVIOUSTRACK = 12;
constexpr int APPCOMMAND_MEDIA_STOP = 13;
constexpr int APPCOMMAND_MEDIA_PLAY_PAUSE = 14;
constexpr int APPCOMMAND_MEDIA_PLAY = 46;
constexpr int APPCOMMAND_MEDIA_PAUSE = 47;
#endif

#ifdef _WIN32
HHOOK nativeKeyboardHook = nullptr;
#endif

ShortcutBinding unsupportedBinding() { return {}; }

ShortcutBinding specialKeyboardBinding(int nativeCode, const std::string& display) {
    ShortcutBinding binding;
    binding.device = ShortcutDevice::Keyboard;
    binding.nativeCode = nativeCode;
    binding.display = display;
    return binding;
}

ShortcutBinding appCommandBinding(int command, const std::string& display) {
    ShortcutBinding binding;
    binding.device = ShortcutDevice::WindowsAppCommand;
    binding.nativeCode = command;
    binding.display = display;
    return binding;
}

#ifdef _WIN32
int appCommandFromVirtualKey(DWORD virtualKey) {
    switch (virtualKey) {
        case VK_BROWSER_BACK:
            return APPCOMMAND_BROWSER_BACKWARD;
        case VK_BROWSER_FORWARD:
            return APPCOMMAND_BROWSER_FORWARD;
        case VK_VOLUME_MUTE:
            return APPCOMMAND_VOLUME_MUTE;
        case VK_VOLUME_DOWN:
            return APPCOMMAND_VOLUME_DOWN;
        case VK_VOLUME_UP:
            return APPCOMMAND_VOLUME_UP;
        case VK_MEDIA_NEXT_TRACK:
            return APPCOMMAND_MEDIA_NEXTTRACK;
        case VK_MEDIA_PREV_TRACK:
            return APPCOMMAND_MEDIA_PREVIOUSTRACK;
        case VK_MEDIA_STOP:
            return APPCOMMAND_MEDIA_STOP;
        case VK_MEDIA_PLAY_PAUSE:
            return APPCOMMAND_MEDIA_PLAY_PAUSE;
        default:
            return 0;
    }
}

LRESULT CALLBACK nativeKeyboardProc(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION && ShortcutCaptureHelper::isCapturing() &&
        (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
        const auto* event = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        const int command = appCommandFromVirtualKey(event->vkCode);
        if (command != 0) {
            const auto binding = ShortcutCaptureHelper::mapWindowsAppCommandEvent(command);
            ShortcutCaptureHelper::publishNativeCapture(binding);
            return 1;
        }
    }
    if (code == HC_ACTION && !ShortcutCaptureHelper::isCapturing() &&
        (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
        const auto* event = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        const int command = appCommandFromVirtualKey(event->vkCode);
        if (command != 0) {
            const auto binding = ShortcutCaptureHelper::mapWindowsAppCommandEvent(command);
            if (ShortcutCaptureHelper::publishNativeShortcut(binding)) return 1;
        }
    }
    return CallNextHookEx(nativeKeyboardHook, code, wParam, lParam);
}
#endif

std::string modifierPrefix(int mods) {
    std::string config;
    if ((mods & GLFW_MOD_CONTROL) != 0) config += "ctrl-";
    if ((mods & GLFW_MOD_ALT) != 0) config += "alt-";
    if ((mods & GLFW_MOD_SHIFT) != 0) config += "shift-";
    if ((mods & GLFW_MOD_SUPER) != 0) config += "meta-";
    return config;
}

std::string keyToken(int key) {
    if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9) return std::string(1, static_cast<char>('0' + key - GLFW_KEY_0));
    if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) return std::string(1, static_cast<char>('a' + key - GLFW_KEY_A));
    if (key >= GLFW_KEY_F1 && key <= GLFW_KEY_F24) return "f" + std::to_string(key - GLFW_KEY_F1 + 1);

    switch (key) {
        case GLFW_KEY_SPACE:
            return "space";
        case GLFW_KEY_APOSTROPHE:
            return "apostrophe";
        case GLFW_KEY_COMMA:
            return "comma";
        case GLFW_KEY_MINUS:
            return "minus";
        case GLFW_KEY_PERIOD:
            return "period";
        case GLFW_KEY_SLASH:
            return "slash";
        case GLFW_KEY_SEMICOLON:
            return "semicolon";
        case GLFW_KEY_EQUAL:
            return "equal";
        case GLFW_KEY_LEFT_BRACKET:
            return "[";
        case GLFW_KEY_BACKSLASH:
            return "backslash";
        case GLFW_KEY_RIGHT_BRACKET:
            return "]";
        case GLFW_KEY_GRAVE_ACCENT:
            return "grave";
        case GLFW_KEY_TAB:
            return "tab";
        case GLFW_KEY_BACKSPACE:
            return "backspace";
        case GLFW_KEY_INSERT:
            return "insert";
        case GLFW_KEY_DELETE:
            return "delete";
        case GLFW_KEY_RIGHT:
            return "right";
        case GLFW_KEY_LEFT:
            return "left";
        case GLFW_KEY_DOWN:
            return "down";
        case GLFW_KEY_UP:
            return "up";
        case GLFW_KEY_PAGE_UP:
            return "pgup";
        case GLFW_KEY_PAGE_DOWN:
            return "pgdn";
        case GLFW_KEY_HOME:
            return "home";
        case GLFW_KEY_END:
            return "end";
        case GLFW_KEY_PAUSE:
            return "pause";
        case GLFW_KEY_MENU:
            return "menu";
        default:
            return {};
    }
}

ShortcutBinding rawScancodeBinding(int scancode) {
    if (scancode == 0) return unsupportedBinding();

    ShortcutBinding binding;
    binding.device = ShortcutDevice::Keyboard;
    binding.nativeCode = scancode;
    binding.display = "Scancode " + std::to_string(scancode);
    return binding;
}

ShortcutBinding mediaScancodeBinding(int scancode) {
    switch (scancode) {
        case SDL_SCANCODE_AUDIONEXT:
            return specialKeyboardBinding(scancode, "Media Next");
        case SDL_SCANCODE_AUDIOPREV:
            return specialKeyboardBinding(scancode, "Media Previous");
        case SDL_SCANCODE_AUDIOSTOP:
            return specialKeyboardBinding(scancode, "Media Stop");
        case SDL_SCANCODE_AUDIOPLAY:
            return specialKeyboardBinding(scancode, "Media Play/Pause");
        case SDL_SCANCODE_AUDIOMUTE:
            return specialKeyboardBinding(scancode, "Mute");
        case SDL_SCANCODE_AC_BACK:
            return specialKeyboardBinding(scancode, "Browser Back");
        case SDL_SCANCODE_AC_FORWARD:
            return specialKeyboardBinding(scancode, "Browser Forward");
        case SDL_SCANCODE_VOLUMEUP:
            return specialKeyboardBinding(scancode, "Volume Up");
        case SDL_SCANCODE_VOLUMEDOWN:
            return specialKeyboardBinding(scancode, "Volume Down");
        default:
            return rawScancodeBinding(scancode);
    }
}
}

ShortcutBinding ShortcutCaptureHelper::mapWindowsKeyboardEvent(int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) return unsupportedBinding();
    if (key == GLFW_KEY_UNKNOWN) return mediaScancodeBinding(scancode);

    const std::string token = keyToken(key);
    if (token.empty()) return unsupportedBinding();

    return ShortcutBindingHelper::parseBinding(modifierPrefix(mods) + token);
}

ShortcutBinding ShortcutCaptureHelper::mapWindowsAppCommandEvent(int command) {
    switch (command) {
        case APPCOMMAND_BROWSER_BACKWARD:
            return appCommandBinding(command, "Browser Back");
        case APPCOMMAND_BROWSER_FORWARD:
            return appCommandBinding(command, "Browser Forward");
        case APPCOMMAND_VOLUME_MUTE:
            return appCommandBinding(command, "Mute");
        case APPCOMMAND_VOLUME_DOWN:
            return appCommandBinding(command, "Volume Down");
        case APPCOMMAND_VOLUME_UP:
            return appCommandBinding(command, "Volume Up");
        case APPCOMMAND_MEDIA_NEXTTRACK:
            return appCommandBinding(command, "Media Next");
        case APPCOMMAND_MEDIA_PREVIOUSTRACK:
            return appCommandBinding(command, "Media Previous");
        case APPCOMMAND_MEDIA_STOP:
            return appCommandBinding(command, "Media Stop");
        case APPCOMMAND_MEDIA_PLAY_PAUSE:
            return appCommandBinding(command, "Media Play/Pause");
        case APPCOMMAND_MEDIA_PLAY:
            return appCommandBinding(command, "Media Play");
        case APPCOMMAND_MEDIA_PAUSE:
            return appCommandBinding(command, "Media Pause");
        default:
            return unsupportedBinding();
    }
}

#ifdef _WIN32
void ShortcutCaptureHelper::startNativeCapture() {
    if (nativeKeyboardHook != nullptr) return;
    nativeKeyboardHook = SetWindowsHookExW(WH_KEYBOARD_LL, nativeKeyboardProc, GetModuleHandleW(nullptr), 0);
}

void ShortcutCaptureHelper::stopNativeCapture() {
    if (nativeKeyboardHook == nullptr) return;
    UnhookWindowsHookEx(nativeKeyboardHook);
    nativeKeyboardHook = nullptr;
}
#endif
