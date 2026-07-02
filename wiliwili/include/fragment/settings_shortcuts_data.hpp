#pragma once

#include <vector>

#include "utils/config_helper.hpp"
#include "utils/shortcut_helper.hpp"

const std::vector<ShortcutAction>& shortcutEditorActions();
brls::BrlsKeyCombination shortcutEditorCurrentKey(ShortcutAction action);
ShortcutBinding shortcutEditorCurrentBinding(ShortcutAction action);
bool shortcutEditorSettingItem(ShortcutAction action, SettingItem& item);
std::string shortcutEditorActionLabelKey(ShortcutAction action);
std::string shortcutEditorDefaultConfigKey(ShortcutAction action);
ShortcutBinding shortcutEditorDefaultBinding(ShortcutAction action);
