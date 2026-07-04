#include "utils/shortcut_capture_helper.hpp"

#include <borealis/core/thread.hpp>

#include <cstdint>
#include <map>
#include <mutex>
#include <utility>

namespace {
std::mutex nativeShortcutMutex;
ShortcutCaptureStatus captureStatus = ShortcutCaptureStatus::Idle;
uint64_t captureGeneration = 0;
uint64_t nativeShortcutGeneration = 0;
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
    {
        std::lock_guard<std::mutex> lock(nativeShortcutMutex);
        captureStatus = ShortcutCaptureStatus::Capturing;
        ++captureGeneration;
    }
    startNativeCapture();
}

void ShortcutCaptureHelper::stopCapture() {
    bool shouldStopNativeCapture = false;
    {
        std::lock_guard<std::mutex> lock(nativeShortcutMutex);
        captureStatus = ShortcutCaptureStatus::Idle;
        ++captureGeneration;
        nativeCaptureCallback     = nullptr;
        shouldStopNativeCapture   = nativeShortcuts.empty();
    }
    if (shouldStopNativeCapture) stopNativeCapture();
}

bool ShortcutCaptureHelper::isCapturing() {
    std::lock_guard<std::mutex> lock(nativeShortcutMutex);
    return captureStatus == ShortcutCaptureStatus::Capturing;
}

void ShortcutCaptureHelper::setNativeCaptureCallback(std::function<void(const ShortcutBinding&)> callback) {
    std::lock_guard<std::mutex> lock(nativeShortcutMutex);
    nativeCaptureCallback = std::move(callback);
}

void ShortcutCaptureHelper::publishNativeCapture(const ShortcutBinding& binding) {
    if (binding.device == ShortcutDevice::Unsupported) return;

    uint64_t generation = 0;
    {
        std::lock_guard<std::mutex> lock(nativeShortcutMutex);
        if (captureStatus != ShortcutCaptureStatus::Capturing || !nativeCaptureCallback) return;
        generation = captureGeneration;
    }

    brls::sync([binding, generation]() {
        std::function<void(const ShortcutBinding&)> callback;
        {
            std::lock_guard<std::mutex> lock(nativeShortcutMutex);
            if (captureStatus != ShortcutCaptureStatus::Capturing || generation != captureGeneration ||
                !nativeCaptureCallback)
                return;
            callback = nativeCaptureCallback;
        }
        callback(binding);
    });
}

void ShortcutCaptureHelper::setNativeDispatchCallback(std::function<bool(ShortcutAction)> callback) {
    std::lock_guard<std::mutex> lock(nativeShortcutMutex);
    nativeDispatchCallback = std::move(callback);
}

void ShortcutCaptureHelper::setNativeShortcut(ShortcutAction action, const ShortcutBinding& binding) {
    if (!isNativeShortcutBinding(binding)) {
        bool shouldStopNativeCapture = false;
        {
            std::lock_guard<std::mutex> lock(nativeShortcutMutex);
            nativeShortcuts.erase(action);
            ++nativeShortcutGeneration;
            shouldStopNativeCapture = captureStatus != ShortcutCaptureStatus::Capturing && nativeShortcuts.empty();
        }
        if (shouldStopNativeCapture) stopNativeCapture();
        return;
    }

    {
        std::lock_guard<std::mutex> lock(nativeShortcutMutex);
        nativeShortcuts[action] = binding;
        ++nativeShortcutGeneration;
    }
    startNativeCapture();
}

bool ShortcutCaptureHelper::publishNativeShortcut(const ShortcutBinding& binding) {
    if (binding.device == ShortcutDevice::Unsupported) return false;

    ShortcutAction action = ShortcutAction::Confirm;
    uint64_t shortcutGeneration = 0;
    {
        std::lock_guard<std::mutex> lock(nativeShortcutMutex);
        if (!nativeDispatchCallback) return false;

        bool matched = false;
        for (const auto& item : nativeShortcuts) {
            if (!matchesNativeShortcut(item.second, binding)) continue;
            action  = item.first;
            matched = true;
            break;
        }
        if (!matched) return false;
        shortcutGeneration = nativeShortcutGeneration;
    }

    brls::sync([action, shortcutGeneration]() {
        std::function<bool(ShortcutAction)> callback;
        {
            std::lock_guard<std::mutex> lock(nativeShortcutMutex);
            if (shortcutGeneration != nativeShortcutGeneration || !nativeDispatchCallback) return;
            callback = nativeDispatchCallback;
        }
        callback(action);
    });
    return true;
}

void ShortcutCaptureHelper::clearNativeShortcuts() {
    bool shouldStopNativeCapture = false;
    {
        std::lock_guard<std::mutex> lock(nativeShortcutMutex);
        nativeShortcuts.clear();
        ++nativeShortcutGeneration;
        shouldStopNativeCapture = captureStatus != ShortcutCaptureStatus::Capturing;
    }
    if (shouldStopNativeCapture) stopNativeCapture();
}

bool ShortcutCaptureHelper::hasNativeShortcuts() {
    std::lock_guard<std::mutex> lock(nativeShortcutMutex);
    return !nativeShortcuts.empty();
}

#ifndef _WIN32
void ShortcutCaptureHelper::startNativeCapture() {}

void ShortcutCaptureHelper::stopNativeCapture() {}
#endif
