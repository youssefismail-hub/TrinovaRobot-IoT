#pragma once
#include <memory>
#include "RobotTypes.h"
#include "IRobotDriver.h"
#include "MotionSafety.h"
#include "RobotStateMachine.h"

class TrinovaRobot {
public:
    bool begin(RobotMode mode);
    void update();

    void forward(uint8_t speed);
    void backward(uint8_t speed);
    void turnLeft(uint8_t speed);
    void turnRight(uint8_t speed);
    void stop();

    bool moveServo(uint8_t channel, uint8_t angle, uint16_t durationMs = 0);

    void emergencyStop();
    bool clearEmergencyStop();

    RobotState   state() const     { return _stateMachine.current(); }
    TrinovaError lastError() const { return _lastError; }

private:
    void applyWheelCommand(WheelCommand cmd, uint8_t speed);

    std::unique_ptr<IRobotDriver> _driver;
    RobotConfig       _config;
    MotionSafety      _safety{_config};
    RobotStateMachine _stateMachine;
    TrinovaError      _lastError   = TrinovaError::NotInitialized;
    bool              _initialized = false;
};