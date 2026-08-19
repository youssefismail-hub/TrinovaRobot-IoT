#include "TrinovaRobot.h"
#include "MockRobotDriver.h"
#include "HardwareRobotDriver.h"
#include "Logger.h"
#include <Arduino.h>

bool TrinovaRobot::begin(RobotMode mode) {
    if (mode == RobotMode::Mock) {
        _driver = std::make_unique<MockRobotDriver>();
    } else {
        _driver = std::make_unique<HardwareRobotDriver>();
    }

    bool ok = _driver->begin();
    _initialized = ok;
    if (ok) {
        _stateMachine.toIdle();
        _lastError = TrinovaError::None;
        Logger::log(LogLevel::Info, "Robot", "begin() OK");
    } else {
        _stateMachine.toError();
        _lastError = TrinovaError::MotorDriverError;
        Logger::log(LogLevel::Error, "Robot", "begin() FAILED");
    }
    return ok;
}

void TrinovaRobot::update() {
    if (!_initialized) return;
    if (_stateMachine.current() == RobotState::Moving && _safety.isTimedOut(millis())) {
        Logger::log(LogLevel::Warn, "Safety", "Command timeout -> forcing stop");
        stop();
    }
}

void TrinovaRobot::applyWheelCommand(WheelCommand cmd, uint8_t speed) {
    if (!_initialized) {
        _lastError = TrinovaError::NotInitialized;
        return;
    }
    if (_safety.isEmergencyStopActive()) {
        _lastError = TrinovaError::EmergencyStopActive;
        Logger::log(LogLevel::Warn, "Safety", "Commande refusee: emergency stop actif");
        return;
    }
    uint8_t safeSpeed = _safety.clampSpeed(speed);
    _driver->setWheelCommand(cmd, safeSpeed);
    _safety.onCommandIssued(millis());
    _stateMachine.toMoving();
    _lastError = TrinovaError::None;
}

void TrinovaRobot::forward(uint8_t speed)   { applyWheelCommand(WheelCommand::Forward, speed); }
void TrinovaRobot::backward(uint8_t speed)  { applyWheelCommand(WheelCommand::Backward, speed); }
void TrinovaRobot::turnLeft(uint8_t speed)  { applyWheelCommand(WheelCommand::TurnLeft, speed); }
void TrinovaRobot::turnRight(uint8_t speed) { applyWheelCommand(WheelCommand::TurnRight, speed); }

void TrinovaRobot::stop() {
    if (!_initialized) { _lastError = TrinovaError::NotInitialized; return; }
    _driver->stopWheels();
    _stateMachine.toIdle();
    _lastError = TrinovaError::None;
}

bool TrinovaRobot::moveServo(uint8_t channel, uint8_t angle, uint16_t /*durationMs*/) {
    if (!_initialized || _safety.isEmergencyStopActive()) {
        _lastError = TrinovaError::EmergencyStopActive;
        return false;
    }
    uint8_t safeAngle = _safety.clampAngle(angle);
    return _driver->setServoAngle(channel, safeAngle);
}

void TrinovaRobot::emergencyStop() {
    if (_driver) _driver->emergencyStop();
    _safety.triggerEmergencyStop();
    _stateMachine.toEmergencyStop();
    _lastError = TrinovaError::None;
    Logger::log(LogLevel::Error, "Safety", "EMERGENCY STOP");
}

bool TrinovaRobot::clearEmergencyStop() {
    bool cleared = _safety.clearEmergencyStop();
    if (cleared) {
        _stateMachine.clearEmergencyStop();
        _lastError = TrinovaError::None;
        Logger::log(LogLevel::Info, "Safety", "Emergency stop leve");
    }
    return cleared;
}