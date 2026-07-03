#include "fragment/settings_shortcuts.hpp"
#include "fragment/settings_shortcuts_data.hpp"

#include "utils/config_helper.hpp"
#include "utils/shortcut_binding.hpp"
#include "utils/shortcut_capture_helper.hpp"

#ifndef TASK3_HELPER_ONLY
#include <borealis/core/application.hpp>
#include <borealis/core/i18n.hpp>
#include <borealis/core/logger.hpp>
#include <borealis/views/cells/cell_radio.hpp>
#include <borealis/views/dialog.hpp>

using namespace brls::literals;
#endif

std::string shortcutEditorBindingText(ShortcutAction action) {
    return ShortcutBindingHelper::formatBinding(shortcutEditorCurrentBinding(action));
}

std::string shortcutEditorCapturePlaceholderText() {
    return "shortcuts/capture_placeholder";
}

ShortcutEditorCaptureState shortcutEditorCaptureState(ShortcutEditorCaptureDevice device) {
    switch (device) {
        case ShortcutEditorCaptureDevice::Keyboard:
            return ShortcutEditorCaptureState::Keyboard;
        case ShortcutEditorCaptureDevice::MediaKey:
        case ShortcutEditorCaptureDevice::Gamepad:
            return ShortcutEditorCaptureState::Unsupported;
    }
    return ShortcutEditorCaptureState::Unsupported;
}

std::string shortcutEditorCaptureStateText(ShortcutEditorCaptureState state) {
    switch (state) {
        case ShortcutEditorCaptureState::Keyboard:
            return "shortcuts/capture_keyboard";
        case ShortcutEditorCaptureState::Unsupported:
            return shortcutEditorCapturePlaceholderText();
    }
    return shortcutEditorCapturePlaceholderText();
}

ShortcutBinding shortcutEditorCaptureBinding(brls::BrlsKeyboardScancode key, short mods, bool pressed) {
    if (!pressed) return {};

    ShortcutBinding binding;
    binding.device = ShortcutDevice::Keyboard;
    binding.key.code = key;
    binding.key.mod  = mods;
    if (ShortcutBindingHelper::bindingConfigKey(binding).empty()) return {};
    return binding;
}

bool shortcutEditorSaveBinding(ShortcutAction action, const ShortcutBinding& binding) {
    if (binding.device != ShortcutDevice::Keyboard) return false;
    const std::string configKey = ShortcutBindingHelper::bindingConfigKey(binding);
    if (configKey.empty()) return false;
    if (!ShortcutBindingHelper::applyBinding(action, binding)) return false;

    SettingItem settingItem{};
    if (!shortcutEditorSettingItem(action, settingItem)) return false;

    ProgramConfig::instance().setSettingItem(settingItem, configKey);
    return true;
}

bool shortcutEditorResetBinding(ShortcutAction action) {
    ShortcutCaptureHelper::stopCapture();
    const ShortcutBinding binding = shortcutEditorDefaultBinding(action);
    if (binding.device != ShortcutDevice::Keyboard) return false;
    return shortcutEditorSaveBinding(action, binding);
}

#ifndef TASK3_HELPER_ONLY
SettingsShortcuts::SettingsShortcuts() {
    this->inflateFromXMLRes("xml/fragment/settings_shortcuts.xml");
    brls::Logger::debug("Fragment SettingsShortcuts: create");

    auto* shortcutList = dynamic_cast<brls::Box*>(this->getView("setting/shortcuts/list"));
    if (!shortcutList) return;

    for (auto action : shortcutEditorActions()) {
        auto* cell = new brls::RadioCell();
        this->updateCellTitle(action, cell);
        cell->registerClickAction([this, action, cell](...) -> bool {
            this->openCaptureDialog(action, cell);
            return true;
        });
        shortcutList->addView(cell);
    }
}

SettingsShortcuts::~SettingsShortcuts() { this->unsubscribeKeyboardCapture(); }

brls::View* SettingsShortcuts::create() { return new SettingsShortcuts(); }

void SettingsShortcuts::updateCellTitle(ShortcutAction action, brls::RadioCell* cell) {
    std::string binding = shortcutEditorBindingText(action);
    if (binding.empty()) binding = brls::getStr("shortcuts/not_set");
    cell->title->setText(brls::getStr(shortcutEditorActionLabelKey(action)) + "    " + binding);
}

void SettingsShortcuts::unsubscribeKeyboardCapture() {
    if (!this->keyboardCaptureSubscribed) return;
    brls::Application::getPlatform()->getInputManager()->getKeyboardKeyStateChanged()->unsubscribe(
        this->keyboardCaptureSubscription);
    this->keyboardCaptureSubscribed = false;
}

void SettingsShortcuts::openCaptureDialog(ShortcutAction action, brls::RadioCell* cell) {
    std::string binding = shortcutEditorBindingText(action);
    if (binding.empty()) binding = brls::getStr("shortcuts/not_set");
    const auto captureState = shortcutEditorCaptureState(ShortcutEditorCaptureDevice::Keyboard);

    auto message = brls::getStr(shortcutEditorActionLabelKey(action)) + "\n\n" +
                   brls::getStr(shortcutEditorCaptureStateText(captureState)) + "\n\n" +
                   brls::getStr("shortcuts/current_binding") + ": " + binding;
    auto* dialog = new brls::Dialog(message);
    this->unsubscribeKeyboardCapture();
    ShortcutCaptureHelper::startCapture();
    this->keyboardCaptureSubscription =
        brls::Application::getPlatform()->getInputManager()->getKeyboardKeyStateChanged()->subscribe(
            [this, action, cell, dialog](brls::KeyState state) {
                if (!ShortcutCaptureHelper::isCapturing()) return;
                const auto captured = shortcutEditorCaptureBinding(state.key, state.mods, state.pressed);
                if (captured.device != ShortcutDevice::Keyboard) return;
                if (!shortcutEditorSaveBinding(action, captured)) return;

                ShortcutCaptureHelper::stopCapture();
                this->updateCellTitle(action, cell);
                dialog->close();
            });
    this->keyboardCaptureSubscribed = true;

    dialog->addButton("shortcuts/reset"_i18n, [this, action, cell, dialog]() {
        this->resetBinding(action, cell);
        this->unsubscribeKeyboardCapture();
        dialog->close();
    });
    dialog->addButton("hints/cancel"_i18n, []() { ShortcutCaptureHelper::stopCapture(); });
    dialog->open();
}

void SettingsShortcuts::resetBinding(ShortcutAction action, brls::RadioCell* cell) {
    (void)shortcutEditorResetBinding(action);
    this->updateCellTitle(action, cell);
}
#endif
