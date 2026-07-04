# Shortcut Branch New-File Restructure — Design

**Date:** 2026-07-03
**Branch:** `yoga-dma` (main branch: `yoga`)
**Author:** collaborative design (brainstorming)

## Goal

Restructure the custom-shortcut-binding work already implemented on `yoga-dma` so that the
branch's footprint on upstream (`yoga`) files is as small as possible, with the feature living
mostly in **new, branch-owned files**. The objective is to minimize merge conflicts when the
owner later merges upstream `yoga` changes into `yoga-dma`.

Behavior of the feature is preserved, with one deliberate, approved trade-off (see
[Trade-offs](#trade-offs)).

## Context

The branch adds an in-app shortcut binding editor (see the existing plan
`docs/superpowers/plans/2026-06-30-custom-shortcut-binding.md`). Relative to `yoga`, the diff
currently touches **28 files**: 9 are already new files (conflict-free), and 19 are in-place
modifications of upstream files.

The conflict risk is concentrated in a few **in-place rewrites of existing functions**, not in
the bulk line count:

| Upstream file | Nature of change | Root cause |
|---|---|---|
| `wiliwili/source/view/video_view.cpp` (`registerCommonActions`) | entire function rewritten (12 action registrations) | live in-session re-binding ("refresh") |
| `wiliwili/source/utils/config_helper.cpp` | 19-line `getSettingItem` → `getShortcutSetting` rewrite; ~22-line dispatch block inserted into the keyboard hook | object-form config reading + base-key dispatch |
| `wiliwili/source/activity/player_base_activity.cpp` | 3 call sites converted to the new API | live re-binding |
| `wiliwili/source/activity/search_activity.cpp` | 1 call site converted | live re-binding |
| `resources/i18n/*/wiliwili.json` (×7) | translation keys added | — |
| `wiliwili/source/utils/shortcut_helper.cpp` (+361) / `.hpp` (+94) | **almost entirely appended** code; only a 4-line `functionMap` edit is in-place | — |

### Verified technical facts

These were confirmed by reading the code and the pinned borealis submodule
(`xfangfang/borealis@5f08b28`):

1. **i18n merges every `.json` per locale, keyed by filename, with fallback to the default
   locale.** `loadLocale` iterates all `*.json` files in a locale directory and stores each under
   a top-level key equal to its filename (minus `.json`). `getRawStr` looks up the current locale,
   then `LOCALE_DEFAULT`, then returns the raw key. → New keys can live in a **new** file
   `resources/i18n/<locale>/shortcuts.json` (top-level namespace `shortcuts/…`), and locales
   without the file fall back automatically. No edit to any existing `wiliwili.json`.
2. **Sources are globbed:** `CMakeLists.txt` uses `file(GLOB_RECURSE MAIN_SRC wiliwili/source/*.cpp)`.
   New `.cpp` files under `wiliwili/source/` are compiled automatically — **no `CMakeLists.txt` edit**.
3. **`ProgramConfig::setSettingItem(SettingItem, T, bool)` is a template** accepting any JSON value,
   and `getSettingItem<std::string>` reads a plain-string value. → If the editor persists a plain
   string, the original `getSettingItem` load path works unchanged, so the 19-line rewrite can be
   reverted.
4. **Arrow keys, `space`, `pgup/pgdn` already exist in the upstream `functionMap`.** Only
   `enter`/`return`/`escape`/`esc` were added by the branch. The new binding parser can handle those
   four itself, so `shortcut_helper.cpp` need not be touched.
5. **The 6 base navigation keys (Confirm/Back/Navigate*) are dispatched through the global keyboard
   hook, reading `getX()` live on every keypress.** They are inherently instant and do not depend on
   the per-view registration machinery. Only the 19 player/tab/search actions used the
   registration/refresh path.

## Constraints & success criteria

- Merging upstream `yoga` into `yoga-dma` afterwards must produce **no conflicts** in the fully
  reverted files, and only trivial/additive touch in the residual mount points.
- The feature keeps working: all 25 actions remain rebindable; base navigation keys remain instant;
  legacy string configs still load; the editor still persists and applies.
- No `CMakeLists.txt` change; no rewrite of general `ProgramConfig` machinery.
- Done as **forward commits** on `yoga-dma` (history not rewritten; each step reversible).
- Desktop build passes: `cmake -B build -DPLATFORM_DESKTOP=ON && make -C build wiliwili -j$(nproc)`.

## Target architecture

### A. Files fully reverted to upstream `yoga` (conflict risk → 0)

- `wiliwili/source/view/video_view.cpp`, `wiliwili/include/view/video_view.hpp`
- `wiliwili/source/activity/player_base_activity.cpp`, `wiliwili/include/activity/player_activity.hpp`
- `wiliwili/source/activity/search_activity.cpp`, `wiliwili/include/activity/search_activity.hpp`
- `wiliwili/source/utils/shortcut_helper.cpp`, `wiliwili/include/utils/shortcut_helper.hpp`
- all 7 `resources/i18n/*/wiliwili.json`

The player/tab/search call sites return to the native `activity->registerAction(ShortcutHelper::getX(), …)`
form. Because `getX()` returns the currently-configured combination, an edited binding is picked up
the next time that view is constructed.

### B. New branch-owned files

New:

- `wiliwili/include/utils/shortcut_binding.hpp` + `wiliwili/source/utils/shortcut_binding.cpp`
  - `enum class ShortcutDevice`, `enum class ShortcutAction`, `struct ShortcutBinding`.
  - `parseBinding` / `formatBinding` / `bindingConfigKey` / `toBrlsKeyCombination` /
    `gamepadBinding` / `gamepadButtonDisplayName` / key display helpers.
  - `applyBinding(action, binding)` — updates the relevant static combination via
    `ShortcutHelper::setX()` (existing 19) or `BaseShortcutHelper::setX()` (6 base keys). No
    registration-refresh (dropped).
  - `enter`/`escape`/`esc`/`return` handled here so `shortcut_helper.cpp` is untouched.
- `wiliwili/include/utils/base_shortcut_helper.hpp` + `wiliwili/source/utils/base_shortcut_helper.cpp`
  - `BaseShortcutHelper` holding the 6 base-key static combinations (via the existing
    `WILI_DECL_SHORTCUT` macro) + getters/setters.
  - `initBaseShortcuts(ProgramConfig&)` — loads the 6 base-key config values (defaults
    `enter`/`escape`/`up`/`down`/`left`/`right`) using the public `getSettingItem`.
  - `dispatch(const brls::KeyState&) -> bool` — the base-key → controller-button mapping
    (the block currently inlined in `config_helper.cpp`), plus its `shortcutMatchesKeyState` /
    `isNativeControllerKeyState` / `dispatchShortcutButtonOnce` helpers. Returns `true` when a
    base-key event was consumed.

Unchanged existing new files:

- `wiliwili/include/fragment/settings_shortcuts.hpp` + `wiliwili/source/fragment/settings_shortcuts.cpp`
- `wiliwili/include/fragment/settings_shortcuts_data.hpp` + `wiliwili/source/fragment/settings_shortcuts_data.cpp`
- `wiliwili/include/utils/shortcut_capture_helper.hpp` + `wiliwili/source/utils/shortcut_capture_helper.cpp`
- `wiliwili/source/utils/shortcut_capture_windows.cpp`
- `resources/xml/fragment/settings_shortcuts.xml`

New i18n:

- `resources/i18n/<locale>/shortcuts.json` for each of the 7 locales the branch already translated
  (`en-US`, `it`, `ja-RYU`, `ja`, `ko`, `zh-Hans`, `zh-Hant`). The existing translated strings are
  **moved verbatim**, re-namespaced from `wiliwili/setting/shortcuts/…` to `shortcuts/…`.

### C. Residual upstream mount points (small, additive only)

These cannot be eliminated because the feature must attach to the upstream settings menu and the
upstream keyboard event loop. All are additive (no rewrites):

| File | Residual edit |
|---|---|
| `wiliwili/source/activity/setting_activity.cpp` | +1 `#include "fragment/settings_shortcuts.hpp"`; change the `btnHotKey` dialog body to `new SettingsShortcuts()` (~2 lines). Entry point for the editor. |
| `wiliwili/source/utils/config_helper.cpp` | +6 `SETTING_MAP` entries (`shortcut_confirm/back/navigate_*`); +1 line `initBaseShortcuts(*this);` in `load()`; +1 line `if (BaseShortcutHelper::dispatch(state)) return;` at the top of the keyboard-state subscription. **Revert** the 19-line `getShortcutSetting` rewrite back to `getSettingItem`. Move the dispatch helpers out to `base_shortcut_helper.cpp`. |
| `wiliwili/include/utils/config_helper.hpp` | +6 `SettingItem` enum values (`SHORTCUT_CONFIRM/BACK/NAVIGATE_UP/DOWN/LEFT/RIGHT`). |

**Decided options** (recommended defaults):

- `resources/xml/activity/setting_activity.xml`: **keep the upstream title** (`tools/tutorial/hot_key`)
  on the row that now opens the editor → **zero XML edit**. (Alternative rejected: a dedicated
  `shortcuts/title` key = 1-line edit.)
- Base-key persistence: **keep** the 6 `SettingItem` enum + `SETTING_MAP` additions (consistent with
  the other shortcuts, negligible conflict risk since they are additive in the `SHORTCUT_*` block).
  (Alternative rejected: string-keyed side store = zero `config_helper.hpp` edit but extra logic in
  new files.)

### Editor persistence change (enables the config revert)

`shortcutEditorSaveBinding` currently writes an object `{"device":"keyboard","key":"<cfg>"}`. Change
it (in the new file `settings_shortcuts.cpp`) to persist the **plain string** `<cfg>` via
`ProgramConfig::setSettingItem(item, configKey)`. This makes the original `getSettingItem<std::string>`
load path sufficient, so the 19-line `config_helper.cpp` rewrite is reverted. Any config previously
written in object form is a non-issue (feature is unreleased); optionally the loader keeps tolerating
both, but the default is string-only for minimal footprint.

## Net effect

From **19 modified upstream files (incl. 4 heavy rewrites)** down to **3 upstream files with additive
edits**: `config_helper.hpp` (+6), `config_helper.cpp` (~+8 net, no rewrite), `setting_activity.cpp`
(~+3). Everything else is new, branch-owned files.

## Trade-offs

- **Live in-session re-binding is dropped for the 19 player/tab/search actions.** After editing a
  binding, it applies the next time the relevant view is constructed (re-enter player, re-open search)
  or after restart. This matches the original plan's Scenario 1 pass condition (edit → restart →
  still works) and is approved by the branch owner. The 6 base navigation keys remain instant because
  they dispatch through the live keyboard hook. In practice the user edits bindings from the Settings
  screen and returns to the player/search afterward, so the view is rebuilt and reads the new value —
  the difference is nearly invisible.

## Verification

1. `lsp`/build: `cmake -B build -DPLATFORM_DESKTOP=ON && make -C build wiliwili -j$(nproc)` succeeds.
2. Merge dry-run: `git merge --no-commit --no-ff yoga` (or a rebase check) reports no conflicts in the
   reverted files.
3. Manual: Settings → open shortcut editor; rebind an action; confirm base nav keys change instantly;
   confirm a player action change is in effect after re-entering the player; restart and confirm
   persistence; confirm a legacy string config still loads.
4. Regression: legacy values (`ctrl-r`, `meta-f`, `pgup`, `shift-pgdn`, `space`, `[`, `]`) still parse
   to the same `BrlsKeyCombination`.

## Out of scope / YAGNI

- No object-form config schema (plain strings only).
- No `ShortcutRegistration`/refresh subsystem (removed).
- No `CMakeLists.txt` changes.
- No new settings row/layout (reuse the existing hot-key row).
- Leftover `TASK3_HELPER_ONLY` test scaffolding in `settings_shortcuts.*` may be cleaned up
  opportunistically but is not required by this design.

## Risks

- **Reverting the three hotspot files must exactly match upstream** to get the zero-conflict benefit;
  use `git checkout yoga -- <file>` as the base, then re-apply nothing. Verify with
  `git diff yoga -- <file>` being empty.
- **Symbol moves** (`ShortcutAction`, `ShortcutBinding`) change includes in the existing new files
  (`settings_shortcuts_data.*`, `settings_shortcuts.*`) from `utils/shortcut_helper.hpp` to
  `utils/shortcut_binding.hpp`, and base-key getters from `ShortcutHelper::` to `BaseShortcutHelper::`.
  These are all branch-owned files, so free to change, but must be updated consistently.
- **Base-key dispatch ordering**: the `dispatch(state)` call must sit at the exact position where the
  block was inlined (before `if (!state.pressed) return;`) to preserve press/release "held" tracking.
