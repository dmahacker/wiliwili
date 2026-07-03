//
// Base navigation shortcuts (Confirm / Back / Navigate*).
//
// Kept separate from ShortcutHelper so the base-key feature lives in
// branch-owned files and upstream shortcut_helper.* stays untouched.
//

#pragma once

#include <string>

#include <borealis/core/actions.hpp>
#include <borealis/core/application.hpp>
#include <borealis/core/input.hpp>

#include "utils/shortcut_helper.hpp"  // for WILI_DECL_SHORTCUT and ShortcutHelper::parseKey

class ProgramConfig;

class BaseShortcutHelper {
public:
    // 从配置加载并应用 6 个基础键（默认 enter/escape/up/down/left/right）
    static void loadFromConfig(ProgramConfig& config);

    // 在键盘事件回调中把命中的基础键转成对应手柄按键。
    // 命中并消费事件时返回 true（调用方应据此提前 return）。
    static bool dispatch(const brls::KeyState& state);

    // 确认基础键快捷键
    WILI_DECL_SHORTCUT(Confirm);
    // 返回基础键快捷键
    WILI_DECL_SHORTCUT(Back);
    // 向上导航基础键快捷键
    WILI_DECL_SHORTCUT(NavigateUp);
    // 向下导航基础键快捷键
    WILI_DECL_SHORTCUT(NavigateDown);
    // 向左导航基础键快捷键
    WILI_DECL_SHORTCUT(NavigateLeft);
    // 向右导航基础键快捷键
    WILI_DECL_SHORTCUT(NavigateRight);

private:
    // 供 WILI_DECL_SHORTCUT 生成的 setter 使用，委托给 ShortcutHelper 的解析实现
    static brls::BrlsKeyCombination parseKey(const std::string& key) { return ShortcutHelper::parseKey(key); }
};
