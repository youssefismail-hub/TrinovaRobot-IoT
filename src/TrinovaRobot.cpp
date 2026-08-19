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