#pragma once
#include "IRobotDriver.h"
#include "DcMotorDriver.h"
#include "Pca9685Driver.h"
#include "ServoDriver.h"
#include "HardwarePins.h"

// Assemble DcMotorDriver + Pca9685Driver + ServoDriver derrière IRobotDriver.
// TrinovaRobot ne connaît que cette classe en mode Hardware — même contrat que MockRobotDriver.
class HardwareRobotDriver : public IRobotDriver {
public:
    HardwareRobotDriver();

    bool begin() override;
    void setWheelCommand(WheelCommand cmd, uint8_t speed) override;
    void stopWheels() override;
    bool setServoAngle(uint8_t channel, uint8_t angle) override;
    void emergencyStop() override;

private:
    DcMotorDriver  _motors;
    Pca9685Driver  _pca;
    ServoDriver    _servo;
    bool           _motorsReady = false;
    bool           _servoReady  = false;
};