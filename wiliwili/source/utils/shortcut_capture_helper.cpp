#include "utils/shortcut_capture_helper.hpp"

#include <utility>

namespace {
ShortcutCaptureStatus captureStatus = ShortcutCaptureStatus::Idle;
std::function<void(const ShortcutBinding&)> nativeCaptureCallback;
}

void ShortcutCaptureHelper::startCapture() {
    captureStatus = ShortcutCaptureStatus::Capturing;
    startNativeCapture();
}

void ShortcutCaptureHelper::stopCapture() {
    captureStatus = ShortcutCaptureStatus::Idle;
    stopNativeCapture();
    nativeCaptureCallback = nullptr;
}

bool ShortcutCaptureHelper::isCapturing() { return captureStatus == ShortcutCaptureStatus::Capturing; }

void ShortcutCaptureHelper::setNativeCaptureCallback(std::function<void(const ShortcutBinding&)> callback) {
    nativeCaptureCallback = std::move(callback);
}

void ShortcutCaptureHelper::publishNativeCapture(const ShortcutBinding& binding) {
    if (!isCapturing() || binding.device == ShortcutDevice::Unsupported || !nativeCaptureCallback) return;
    nativeCaptureCallback(binding);
}

#ifndef _WIN32
void ShortcutCaptureHelper::startNativeCapture() {}

void ShortcutCaptureHelper::stopNativeCapture() {}
#endif
