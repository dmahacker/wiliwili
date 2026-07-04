#include "utils/shortcut_capture_helper.hpp"

#include <borealis/core/thread.hpp>

#include <cstdint>
#include <map>
#include <utility>

namespace {
ShortcutCaptureStatus captureStatus = ShortcutCaptureStatus::Idle;
uint64_t captureGeneration = 0;
std::function<void(const ShortcutBinding&)> nativeCaptureCallback;
std::function<bool(ShortcutAction)> nativeDispatchCallback;
std::map<ShortcutAction, ShortcutBinding> nativeShortcuts;

bool isNativeShortcutBinding(const ShortcutBinding& binding) {
    if (binding.device == ShortcutDevice::WindowsAppCommand) return binding.nativeCode > 0;
    if (binding.device == ShortcutDevice::WindowsRawHid) return binding.nativeCode > 0;
    return binding.device == ShortcutDevice::Keyboard && binding.key.code == brls::BRLS_KBD_KEY_UNKNOWN &&
           binding.nativeCode > 0;
}

bool matchesNativeShortcut(const ShortcutBinding& lhs, const ShortcutBinding& rhs) {
    return lhs.device == rhs.device && lhs.nativeCode > 0 && lhs.nativeCode == rhs.nativeCode;
}
}

void ShortcutCaptureHelper::startCapture() {
    captureStatus = ShortcutCaptureStatus::Capturing;
    ++captureGeneration;
    startNativeCapture();
}

void ShortcutCaptureHelper::stopCapture() {
    captureStatus = ShortcutCaptureStatus::Idle;
    ++captureGeneration;
    if (!hasNativeShortcuts()) stopNativeCapture();
    nativeCaptureCallback = nullptr;
}

bool ShortcutCaptureHelper::isCapturing() { return captureStatus == ShortcutCaptureStatus::Capturing; }

void ShortcutCaptureHelper::setNativeCaptureCallback(std::function<void(const ShortcutBinding&)> callback) {
    nativeCaptureCallback = std::move(callback);
}

void ShortcutCaptureHelper::publishNativeCapture(const ShortcutBinding& binding) {
    if (!isCapturing() || binding.device == ShortcutDevice::Unsupported || !nativeCaptureCallback) return;
    const uint64_t generation = captureGeneration;
    brls::sync([binding, generation]() {
        if (!isCapturing() || generation != captureGeneration || !nativeCaptureCallback) return;
        nativeCaptureCallback(binding);
    });
}

void ShortcutCaptureHelper::setNativeDispatchCallback(std::function<bool(ShortcutAction)> callback) {
    nativeDispatchCallback = std::move(callback);
}

void ShortcutCaptureHelper::setNativeShortcut(ShortcutAction action, const ShortcutBinding& binding) {
    if (!isNativeShortcutBinding(binding)) {
        nativeShortcuts.erase(action);
        if (!isCapturing() && !hasNativeShortcuts()) stopNativeCapture();
        return;
    }

    nativeShortcuts[action] = binding;
    startNativeCapture();
}

bool ShortcutCaptureHelper::publishNativeShortcut(const ShortcutBinding& binding) {
    if (binding.device == ShortcutDevice::Unsupported || !nativeDispatchCallback) return false;

    for (const auto& item : nativeShortcuts) {
        if (matchesNativeShortcut(item.second, binding)) return nativeDispatchCallback(item.first);
    }
    return false;
}

void ShortcutCaptureHelper::clearNativeShortcuts() {
    nativeShortcuts.clear();
    if (!isCapturing()) stopNativeCapture();
}

bool ShortcutCaptureHelper::hasNativeShortcuts() { return !nativeShortcuts.empty(); }

#ifndef _WIN32
void ShortcutCaptureHelper::startNativeCapture() {}

void ShortcutCaptureHelper::stopNativeCapture() {}
#endif
