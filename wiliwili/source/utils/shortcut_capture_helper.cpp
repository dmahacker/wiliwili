#include "utils/shortcut_capture_helper.hpp"

namespace {
ShortcutCaptureStatus captureStatus = ShortcutCaptureStatus::Idle;
}

void ShortcutCaptureHelper::startCapture() { captureStatus = ShortcutCaptureStatus::Capturing; }

void ShortcutCaptureHelper::stopCapture() { captureStatus = ShortcutCaptureStatus::Idle; }

bool ShortcutCaptureHelper::isCapturing() { return captureStatus == ShortcutCaptureStatus::Capturing; }
