# Custom Shortcut Binding Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add an in-app shortcut binding editor that starts with Windows keyboard shortcuts, then Windows media keys, then gamepad bindings, while preserving existing shortcut behavior.

**Architecture:** Keep shortcut actions in the existing `ShortcutHelper`/`registerAction` flow. Add a common binding model above platform input events, then feed it from Windows keyboard/media adapters and later gamepad/platform adapters. Settings UI edits persisted `ProgramConfig` shortcut values and reapplies them through `ShortcutHelper`.

**Tech Stack:** C++17, borealis XML UI, `ProgramConfig`, `ShortcutHelper`, nlohmann/json config, GLFW/SDL2 desktop input, Win32 `WM_APPCOMMAND` for Windows media keys.

## Global Constraints

- Implement Windows-first support in this priority order: ordinary keyboard keys, Windows media/special keys, gamepad bindings, other platforms by capability.
- Preserve legacy string shortcut config values such as `ctrl-r`, `meta-f`, `pgup`, `space`, `[`, and `]`.
- Do not rewrite all input handling; adapt the existing `ShortcutHelper` and `registerAction` path.
- Keep non-Windows and console platforms capability-gated instead of promising unsupported keys.
- Existing settings patterns are XML-driven layouts under `resources/xml/` plus `BRLS_BIND` fields in C++.
- Current editable settings live in `SettingActivity`; current hot-key UI is only an informational dialog at `resources/xml/fragment/settings_hot_keys.xml`.

---

## Scenarios

### Scenario 1: Windows Keyboard Happy Path

**Pass condition:** A Windows desktop user opens Settings, chooses the Search shortcut row, presses `Ctrl+F5`, sees `Ctrl+F5` displayed, restarts the app, and Search still responds to `Ctrl+F5`.

**Real surface:** Desktop app settings UI and search action.

**Test target:** Unit coverage for parsing/serializing `ctrl-f5`; integration/helper coverage for loading it from `ProgramConfig` and applying `ShortcutHelper::setSearch`.

### Scenario 2: Unsupported Key Edge

**Pass condition:** When a platform cannot observe a pressed key, the capture dialog displays an unsupported message and the existing binding remains unchanged.

**Real surface:** Shortcut capture dialog.

**Test target:** Unit coverage for unsupported binding parse/capture result not overwriting the stored binding.

### Scenario 3: Legacy Config Regression

**Pass condition:** Existing configs containing string shortcuts still load and register the same `BrlsKeyCombination` values as before.

**Real surface:** App startup config load.

**Test target:** Unit coverage for legacy values: `ctrl-r`, `meta-r`, `pgup`, `shift-pgdn`, `space`, `[`, `]`.

### Scenario 4: Windows Media Key Capability

**Pass condition:** On Windows, media play/pause can be captured either through SDL media scancodes when SDL is active or through `WM_APPCOMMAND` when GLFW cannot expose the key.

**Real surface:** Desktop app capture dialog and player play/pause action.

**Test target:** Unit coverage for mapping `SDL_SCANCODE_AUDIOPLAY` and `APPCOMMAND_MEDIA_PLAY_PAUSE` into the common binding model.

---

## File Structure

### Core Shortcut Model

- Modify: `wiliwili/include/utils/shortcut_helper.hpp`
  - Add a binding model type and public parser/serializer/apply functions.
- Modify: `wiliwili/source/utils/shortcut_helper.cpp`
  - Keep legacy string parser, add structured binding parse/format, and map bindings to `brls::BrlsKeyCombination` where possible.
- Modify: `wiliwili/include/utils/config_helper.hpp`
  - Keep existing `SettingItem::SHORTCUT_*` values; do not add a separate setting item per device.
- Modify: `wiliwili/source/utils/config_helper.cpp`
  - Load legacy strings and new binding objects from the same shortcut keys.

### Settings UI

- Modify: `resources/xml/activity/setting_activity.xml`
  - Add a shortcut editor entry near the existing UI/keymap settings or replace the current Tools hot-key help row with an editor entry.
- Modify: `wiliwili/include/activity/setting_activity.hpp`
  - Bind the new shortcut editor row if it is added to the main settings layout.
- Modify: `wiliwili/source/activity/setting_activity.cpp`
  - Open the shortcut editor dialog or fragment from the settings row.
- Create: `resources/xml/fragment/settings_shortcuts.xml`
  - Shortcut editor layout using existing borealis cell patterns.
- Create: `wiliwili/include/fragment/settings_shortcuts.hpp`
  - Editor view class and cell bindings.
- Create: `wiliwili/source/fragment/settings_shortcuts.cpp`
  - Action list, capture dialog, reset-to-default, persistence, and capability state.

### Platform Input Capture

- Create: `wiliwili/include/utils/shortcut_capture_helper.hpp`
  - Common capture result interface independent of GLFW/SDL/Win32.
- Create: `wiliwili/source/utils/shortcut_capture_helper.cpp`
  - Non-platform-specific capture state and binding formatting.
- Create: `wiliwili/source/utils/shortcut_capture_windows.cpp`
  - Windows ordinary key/media key adapter guarded with `_WIN32`.

### Localized Text

- Modify: `resources/i18n/en-US/wiliwili.json`
- Modify: `resources/i18n/zh-Hans/wiliwili.json`
- Modify other locale files only with English fallback text if the project requires every locale key to exist.

---

## Task 1: Binding Model

**Files:**
- Modify: `wiliwili/include/utils/shortcut_helper.hpp`
- Modify: `wiliwili/source/utils/shortcut_helper.cpp`

**Interfaces:**
- Produces: `ShortcutBinding`, `ShortcutAction`, `ShortcutHelper::parseBinding`, `ShortcutHelper::formatBinding`, `ShortcutHelper::toBrlsKeyCombination`.
- Consumes: existing `ShortcutHelper::parseKey` and `brls::BrlsKeyCombination`.

- [ ] **Step 1: Write failing parser tests or a temporary test harness**

If the repo has no active test target, create a temporary local harness under `/tmp` during development that includes only `shortcut_helper.cpp` and verifies parser behavior. Do not commit the harness unless the repo already has a test target.

Required assertions:

```cpp
// Given: a legacy shortcut string
// When: it is parsed as a binding
// Then: it produces a keyboard binding with ctrl modifier and F5 code
assert(ShortcutHelper::parseBinding("ctrl-f5").format() == "Ctrl+F5");

// Given: an existing punctuation shortcut
// When: it is parsed as a binding
// Then: it preserves the old key identity
assert(ShortcutHelper::parseBinding("]").format() == "]");
```

- [ ] **Step 2: Run the failing parser proof**

Run the smallest available command for the test harness or unit target. Expected result before implementation: compile failure or assertion failure because `parseBinding` does not exist.

- [ ] **Step 3: Add the binding model**

Add a type that can represent exactly these device classes first:

```cpp
enum class ShortcutDevice {
    Keyboard,
    WindowsAppCommand,
    Gamepad,
    Unsupported,
};

struct ShortcutBinding {
    ShortcutDevice device = ShortcutDevice::Unsupported;
    brls::BrlsKeyCombination key{brls::BRLS_KBD_KEY_UNKNOWN};
    int nativeCode = 0;
    std::string display;
};
```

Keep `parseKey` available for old call sites. Add new methods without changing every action registration yet.

- [ ] **Step 4: Verify legacy parsing still passes**

Run the parser proof again. Expected: all legacy assertions pass.

---

## Task 2: Config Compatibility

**Files:**
- Modify: `wiliwili/source/utils/config_helper.cpp`
- Modify: `wiliwili/include/utils/shortcut_helper.hpp`
- Modify: `wiliwili/source/utils/shortcut_helper.cpp`

**Interfaces:**
- Consumes: `ShortcutHelper::parseBinding` from Task 1.
- Produces: shortcut config load path that accepts both JSON string and JSON object values.

- [ ] **Step 1: Write failing compatibility tests or harness assertions**

Required cases:

```cpp
// Given: legacy string config shortcut_refresh = "ctrl-r"
// When: ProgramConfig initializes shortcuts
// Then: ShortcutHelper::getRefresh() equals parseKey("ctrl-r")

// Given: object config shortcut_refresh = {"device":"keyboard","key":"ctrl-r"}
// When: ProgramConfig initializes shortcuts
// Then: ShortcutHelper::getRefresh() equals parseKey("ctrl-r")
```

- [ ] **Step 2: Run compatibility test red**

Expected: object-form config fails because current `getSettingItem(..., std::string{})` expects a string.

- [ ] **Step 3: Add a focused shortcut config loader**

Do not change general `ProgramConfig::getSettingItem`. Add a private/static helper in `config_helper.cpp` near shortcut initialization that reads one shortcut setting and returns a legacy-compatible binding string or `ShortcutBinding`.

- [ ] **Step 4: Reapply shortcuts after config load**

Keep the current default list from `ProgramConfig::init()`:

```cpp
ShortcutHelper::setRefresh(...);
ShortcutHelper::setSearch(...);
ShortcutHelper::setLast(...);
ShortcutHelper::setNext(...);
ShortcutHelper::setLastSub(...);
ShortcutHelper::setNextSub(...);
ShortcutHelper::setVolumeUp(...);
ShortcutHelper::setVolumeDown(...);
ShortcutHelper::setDanmaku(...);
ShortcutHelper::setVideoProfile(...);
ShortcutHelper::setVideoQuality(...);
ShortcutHelper::setVideoSpeed(...);
ShortcutHelper::setPlaylist(...);
ShortcutHelper::setSetting(...);
ShortcutHelper::setVideoSpeedUp(...);
ShortcutHelper::setForward(...);
ShortcutHelper::setRewind(...);
ShortcutHelper::setVideoOsd(...);
ShortcutHelper::setVideoPause(...);
```

- [ ] **Step 5: Verify legacy config regression**

Expected: all legacy strings still produce the same `BrlsKeyCombination` values.

---

## Task 3: Settings Editor UI

**Files:**
- Modify: `resources/xml/activity/setting_activity.xml`
- Modify: `wiliwili/include/activity/setting_activity.hpp`
- Modify: `wiliwili/source/activity/setting_activity.cpp`
- Create: `resources/xml/fragment/settings_shortcuts.xml`
- Create: `wiliwili/include/fragment/settings_shortcuts.hpp`
- Create: `wiliwili/source/fragment/settings_shortcuts.cpp`

**Interfaces:**
- Consumes: `ShortcutBinding` and config compatibility from Tasks 1-2.
- Produces: `SettingsShortcuts` view opened from Settings.

- [ ] **Step 1: Write failing UI-helper tests where possible**

Separate non-visual logic from the view: action list, display labels, reset defaults, unsupported-state text. Test those pure helpers first.

- [ ] **Step 2: Add the XML view**

Use existing settings layout style: `brls:Box`, `brls:Header`, and per-action rows. Keep it minimal: action name, current binding text, and click-to-capture behavior.

- [ ] **Step 3: Add the C++ fragment**

`SettingsShortcuts` owns:

```cpp
class SettingsShortcuts : public brls::Box {
public:
    SettingsShortcuts();
    static brls::View* create();
private:
    void openCaptureDialog(ShortcutAction action);
    void resetBinding(ShortcutAction action);
};
```

- [ ] **Step 4: Wire SettingsActivity**

Change the current hot-key row action from help-only to opening the editor dialog. Keep help text inside the editor or behind a help button.

- [ ] **Step 5: Manual UI verification**

Run the desktop app if buildable. Verify: settings opens, shortcut editor opens, rows are focusable, current bindings display, cancel leaves values unchanged.

---

## Task 4: Windows Ordinary Keyboard Capture

**Files:**
- Create: `wiliwili/include/utils/shortcut_capture_helper.hpp`
- Create: `wiliwili/source/utils/shortcut_capture_helper.cpp`
- Create: `wiliwili/source/utils/shortcut_capture_windows.cpp`

**Interfaces:**
- Consumes: `ShortcutBinding`.
- Produces: capture result for ordinary keyboard keys and modifiers.

- [ ] **Step 1: Write failing mapping tests**

Required cases:

```cpp
// Given: Windows key event Ctrl+F5
// When: it is mapped to ShortcutBinding
// Then: display is Ctrl+F5 and device is Keyboard

// Given: key is GLFW_KEY_UNKNOWN with a scancode
// When: it is mapped
// Then: binding stores the native scancode and marks display as platform-specific
```

- [ ] **Step 2: Implement common capture state**

Provide a start/stop capture API that the UI can call without knowing platform backend details.

- [ ] **Step 3: Implement Windows ordinary key mapping**

Use GLFW key/scancode/action/mods when GLFW is active. Respect GLFW docs: key callbacks are the reliable capture path; polling can miss transitions.

- [ ] **Step 4: Verify capture manually on Windows**

Build Windows desktop and bind: `Ctrl+F5`, `F13` if available, `Space`, `[`, `]`, arrow keys.

---

## Task 5: Windows Media/Special Key Capture

**Files:**
- Modify: `wiliwili/source/utils/shortcut_capture_windows.cpp`
- Modify build files only if required after locating the actual desktop window hook in the full borealis source.

**Interfaces:**
- Consumes: `ShortcutBinding` with `ShortcutDevice::WindowsAppCommand`.
- Produces: Windows media/app-command capture results.

- [ ] **Step 1: Write failing mapping tests**

Required cases:

```cpp
// Given: APPCOMMAND_MEDIA_PLAY_PAUSE
// When: it is mapped to ShortcutBinding
// Then: display is Media Play/Pause

// Given: SDL_SCANCODE_AUDIOPLAY
// When: SDL2 is active and it is mapped
// Then: display is Media Play/Pause
```

- [ ] **Step 2: Add SDL media scancode mapping**

Map SDL scancodes including `SDL_SCANCODE_AUDIONEXT`, `SDL_SCANCODE_AUDIOPREV`, `SDL_SCANCODE_AUDIOSTOP`, `SDL_SCANCODE_AUDIOPLAY`, `SDL_SCANCODE_AUDIOMUTE`, and browser/app keys supported by SDL.

- [ ] **Step 3: Add Win32 app-command mapping**

Map `WM_APPCOMMAND` values including `APPCOMMAND_MEDIA_PLAY_PAUSE`, `APPCOMMAND_MEDIA_PLAY`, `APPCOMMAND_MEDIA_PAUSE`, previous/next/stop, volume up/down/mute.

- [ ] **Step 4: Attach at the narrowest Windows window hook**

Locate the actual Win32 window procedure in the full borealis checkout. If it is not present in this repository checkout, document the required patch location and keep app-side code behind `_WIN32` capability flags.

- [ ] **Step 5: Manual Windows verification**

Use a keyboard with media keys. Verify capture dialog sees play/pause and does not trigger playback while capturing.

---

## Task 6: Gamepad Binding Support

**Files:**
- Modify: `wiliwili/include/utils/shortcut_helper.hpp`
- Modify: `wiliwili/source/utils/shortcut_helper.cpp`
- Modify: `wiliwili/source/fragment/settings_shortcuts.cpp`

**Interfaces:**
- Consumes: common binding model.
- Produces: gamepad binding records and capability-gated UI state.

- [ ] **Step 1: Write failing gamepad model tests**

Required case:

```cpp
// Given: controller button BUTTON_X
// When: it is converted to ShortcutBinding
// Then: device is Gamepad and display is X
```

- [ ] **Step 2: Add gamepad binding representation**

Represent controller buttons separately from keyboard scancodes. Do not change existing gamepad action registrations yet.

- [ ] **Step 3: Add UI capability state**

If a platform cannot capture gamepad buttons, show the existing binding and a disabled/unsupported capture state.

- [ ] **Step 4: Manual regression check**

Verify existing controller buttons still operate without rebinding.

---

## Task 7: Integration and Capability Gating

**Files:**
- Modify: `wiliwili/source/fragment/settings_shortcuts.cpp`
- Modify: `wiliwili/source/utils/config_helper.cpp`
- Modify: `wiliwili/source/utils/shortcut_helper.cpp`
- Modify: `wiliwili/source/view/video_view.cpp` only if new binding application requires a small adapter.

**Interfaces:**
- Consumes: UI, config, and platform capture tasks.
- Produces: end-to-end shortcut edit, save, apply, and reset workflow.

- [ ] **Step 1: Write failing integration-helper tests**

Required cases:

```cpp
// Given: changed binding for VideoPause
// When: binding is saved
// Then: ProgramConfig stores it and ShortcutHelper::getVideoPause returns the updated key

// Given: platform capability excludes media keys
// When: opening media capture
// Then: UI returns unsupported without changing config
```

- [ ] **Step 2: Apply changed bindings immediately**

After saving a setting, call the matching `ShortcutHelper::set*` path or a new action-based apply function.

- [ ] **Step 3: Preserve unsupported platform behavior**

On Switch/PSV/PS4 and unsupported desktop backends, show bindings but disable capture for unavailable device classes.

- [ ] **Step 4: Manual end-to-end verification**

Verify bind, rebind, reset, cancel, restart persistence, and legacy config load.

---

## Task 8: Final Verification

**Files:**
- No required source changes unless verification finds defects.

**Interfaces:**
- Consumes all prior tasks.
- Produces release-ready evidence.

- [ ] **Step 1: Run changed-file diagnostics**

Run `lsp_diagnostics` on every changed C++ header/source file.

- [ ] **Step 2: Run build**

Run the local desktop build command available in this checkout. On Linux:

```sh
cmake -B build -DPLATFORM_DESKTOP=ON
make -C build wiliwili -j$(nproc)
```

- [ ] **Step 3: Run scenario checks**

Capture evidence for all scenarios listed above.

- [ ] **Step 4: Visual/manual QA**

Open Settings, inspect the shortcut editor, capture screenshots or terminal/app logs if available, then close all spawned processes.

---

## Execution Order

1. Task 1
2. Task 2
3. Tasks 3, 4, and 5 in parallel if different workers are available
4. Task 6
5. Task 7
6. Task 8

## Self-Review

- Spec coverage: The plan covers common model, Windows keyboard, Windows media keys, gamepad, and other platforms by capability.
- Placeholder scan: No `TBD`/`TODO` placeholders are present; platform hook uncertainty is captured as an execution step because the borealis backend source is not present in this checkout.
- Type consistency: `ShortcutBinding`, `ShortcutDevice`, and `ShortcutAction` are consistently named across tasks.
