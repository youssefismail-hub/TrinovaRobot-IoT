#include "MotionSafety.h"

MotionSafety::MotionSafety(const RobotConfig& config) : _config(config) {}

uint8_t MotionSafety::clampSpeed(uint8_t speed) const {
    return (speed > 100) ? 100 : speed;
}

uint8_t MotionSafety::clampAngle(uint8_t angle) const {
    if (angle > _config.maxAngle) return _config.maxAngle;
    if (angle < _config.minAngle) return _config.minAngle;
    return angle;
}

void MotionSafety::onCommandIssued(uint32_t nowMs) {
    _lastCommandMs = nowMs;
    _hasCommand = true;
}

bool MotionSafety::isTimedOut(uint32_t nowMs) const {
    if (!_hasCommand) return false;
    return (nowMs - _lastCommandMs) >= _config.commandTimeoutMs;
}

void MotionSafety::triggerEmergencyStop() {
    _emergencyActive = true;
}

bool MotionSafety::clearEmergencyStop() {
    if (!_emergencyActive) return false;
    _emergencyActive = false;
    _hasCommand = false;
    return true;
}