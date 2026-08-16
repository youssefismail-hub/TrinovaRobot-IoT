#pragma once
#include <cstdint>

// Interface commune à tous les pilotes (Mock, DC réel, Servo réel)
enum class WheelCommand {
    Forward,
    Backward,
    TurnLeft,
    TurnRight,
    Stop
};

class IRobotDriver {
public:
    virtual ~IRobotDriver() = default;

    virtual bool begin() = 0;
    virtual void setWheelCommand(WheelCommand cmd, uint8_t speed) = 0;
    virtual void stopWheels() = 0;
    virtual bool setServoAngle(uint8_t channel, uint8_t angle) = 0;
    virtual void emergencyStop() = 0;
};