#include "TrinovaRobot.h"
#include "MockRobotDriver.h"

bool TrinovaRobot::begin(RobotMode mode) {
    if (mode == RobotMode::Mock) {
        _driver = std::make_unique<MockRobotDriver>();
    } else {
        // Hardware réel arrivera à l'étape 3 (DcMotorDriver + PCA9685)
        _lastError = TrinovaError::NotInitialized;
        _state = RobotState::Error;
        return false;
    }

    bool ok = _driver->begin();
    _initialized = ok;
    _state = ok ? RobotState::Idle : RobotState::Error;
    _lastError = ok ? TrinovaError::None : TrinovaError::MotorDriverError;
    return ok;
}

void TrinovaRobot::update() {
    // Le timeout de commande sera branché ici à l'étape 2 (MotionSafety)
}

uint8_t TrinovaRobot::clampSpeed(uint8_t speed) const {
    if (speed > 100) return 100;
    return speed;
}

void TrinovaRobot::applyWheelCommand(WheelCommand cmd, uint8_t speed) {
    if (!_initialized) {
        _lastError = TrinovaError::NotInitialized;
        return;
    }
    if (_state == RobotState::EmergencyStop) {
        _lastError = TrinovaError::EmergencyStopActive;
        return;
    }
    uint8_t safeSpeed = clampSpeed(speed);
    _driver->setWheelCommand(cmd, safeSpeed);
    _state = (cmd == WheelCommand::Stop) ? RobotState::Idle : RobotState::Moving;
    _lastError = TrinovaError::None;
}

void TrinovaRobot::forward(uint8_t speed)  { applyWheelCommand(WheelCommand::Forward, speed); }
void TrinovaRobot::backward(uint8_t speed) { applyWheelCommand(WheelCommand::Backward, speed); }
void TrinovaRobot::turnLeft(uint8_t speed) { applyWheelCommand(WheelCommand::TurnLeft, speed); }
void TrinovaRobot::turnRight(uint8_t speed){ applyWheelCommand(WheelCommand::TurnRight, speed); }

void TrinovaRobot::stop() {
    if (!_initialized) { _lastError = TrinovaError::NotInitialized; return; }
    _driver->stopWheels();
    _state = RobotState::Idle;
    _lastError = TrinovaError::None;
}

bool TrinovaRobot::moveServo(uint8_t channel, uint8_t angle, uint16_t /*durationMs*/) {
    if (!_initialized || _state == RobotState::EmergencyStop) {
        _lastError = TrinovaError::EmergencyStopActive;
        return false;
    }
    uint8_t safeAngle = angle;
    if (safeAngle > _config.maxAngle) safeAngle = _config.maxAngle;
    if (safeAngle < _config.minAngle) safeAngle = _config.minAngle;
    return _driver->setServoAngle(channel, safeAngle);
}

void TrinovaRobot::emergencyStop() {
    if (_driver) _driver->emergencyStop();
    _state = RobotState::EmergencyStop;
    _lastError = TrinovaError::None;
}

bool TrinovaRobot::clearEmergencyStop() {
    if (_state != RobotState::EmergencyStop) return false;
    _state = RobotState::Idle;
    _lastError = TrinovaError::None;
    return true;
}