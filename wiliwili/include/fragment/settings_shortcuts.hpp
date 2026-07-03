#pragma once

#include <string>
#include <vector>

#include <borealis/core/box.hpp>

#ifndef TASK3_HELPER_ONLY
#include <borealis/core/input.hpp>
#endif

#include "utils/shortcut_binding.hpp"

#ifndef TASK3_HELPER_ONLY
namespace brls {
class RadioCell;
}
#endif

enum class ShortcutEditorCaptureDevice {
    Keyboard,
    MediaKey,
    Gamepad,
};

enum class ShortcutEditorCaptureState {
    Keyboard,
    Unsupported,
};

const std::vector<ShortcutAction>& shortcutEditorActions();
std::string shortcutEditorActionLabelKey(ShortcutAction action);
std::string shortcutEditorBindingText(ShortcutAction action);
std::string shortcutEditorCapturePlaceholderText();
ShortcutEditorCaptureState shortcutEditorCaptureState(ShortcutEditorCaptureDevice device);
std::string shortcutEditorCaptureStateText(ShortcutEditorCaptureState state);
ShortcutBinding shortcutEditorCaptureBinding(brls::BrlsKeyboardScancode key, short mods, bool pressed);
bool shortcutEditorSaveBinding(ShortcutAction action, const ShortcutBinding& binding);
bool shortcutEditorResetBinding(ShortcutAction action);

class SettingsShortcuts : public brls::Box {
public:
    SettingsShortcuts();
#ifndef TASK3_HELPER_ONLY
    ~SettingsShortcuts();
#endif
    static brls::View* create();

private:
#ifndef TASK3_HELPER_ONLY
    void openCaptureDialog(ShortcutAction action, brls::RadioCell* cell);
    void resetBinding(ShortcutAction action, brls::RadioCell* cell);
    void updateCellTitle(ShortcutAction action, brls::RadioCell* cell);
    void unsubscribeKeyboardCapture();

    brls::Event<brls::KeyState>::Subscription keyboardCaptureSubscription{};
    bool keyboardCaptureSubscribed = false;
#endif
};
