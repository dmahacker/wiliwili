//
// Custom shortcut binding model (branch-owned; kept out of ShortcutHelper to
// minimize churn against upstream shortcut_helper.*).
//

#pragma once

#include <string>

#include <borealis/core/actions.hpp>
#include <borealis/core/application.hpp>

enum class ShortcutDevice {
    Keyboard,
    WindowsAppCommand,
    Gamepad,
    Unsupported,
};

enum class ShortcutAction {
    Confirm,
    Back,
    NavigateUp,
    NavigateDown,
    NavigateLeft,
    NavigateRight,
    Refresh,
    Search,
    Last,
    Next,
    LastSub,
    NextSub,
    VolumeUp,
    VolumeDown,
    VideoProfile,
    Danmaku,
    Playlist,
    Forward,
    Rewind,
    Setting,
    VideoQuality,
    VideoSpeed,
    VideoSpeedUp,
    VideoOsd,
    VideoPause,
};

struct ShortcutBinding {
    ShortcutDevice device = ShortcutDevice::Unsupported;
    brls::BrlsKeyCombination key{brls::BRLS_KBD_KEY_UNKNOWN};
    int nativeCode = 0;
    std::string display;

    std::string format() const;
};

class ShortcutBindingHelper {
public:
    static ShortcutBinding parseBinding(const std::string& config);
    static ShortcutBinding gamepadBinding(int button);
    static std::string gamepadButtonDisplayName(int button);
    static std::string formatBinding(const ShortcutBinding& binding);
    static std::string bindingConfigKey(const ShortcutBinding& binding);
    static brls::BrlsKeyCombination toBrlsKeyCombination(const ShortcutBinding& binding);
    static bool applyBinding(ShortcutAction action, const ShortcutBinding& binding);
};
