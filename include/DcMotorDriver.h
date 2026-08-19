#pragma once
#include <cstdint>
#include "HardwarePins.h"

class DcMotorDriver {
public:
    enum class Direction { Forward, Backward, Stop };

    explicit DcMotorDriver(const MotorPins& pins, uint16_t pwmFrequency = 20000);

    bool begin();
    void setLeftMotor(Direction dir, uint8_t speed);
    void setRightMotor(Direction dir, uint8_t speed);
    void stopAll();

private:
    void applyMotor(uint8_t in1, uint8_t in2, uint8_t ledcChannel, Direction dir, uint8_t speed);
    uint8_t speedToDuty(uint8_t speed) const;

    MotorPins _pins;
    uint16_t  _pwmFrequency;
    bool      _initialized = false;
};