#pragma once
#include "IRobotDriver.h"

class MockRobotDriver : public IRobotDriver {
public:
    bool begin() override;
    void setWheelCommand(WheelCommand cmd, uint8_t speed) override;
    void stopWheels() override;
    bool setServoAngle(uint8_t channel, uint8_t angle) override;
    void emergencyStop() override;

    // Accès pour les tests
    WheelCommand lastWheelCommand() const { return _lastCmd; }
    uint8_t      lastSpeed()        const { return _lastSpeed; }
    bool         emergencyActive()  const { return _emergencyActive; }

private:
    WheelCommand _lastCmd          = WheelCommand::Stop;
    uint8_t      _lastSpeed        = 0;
    bool         _emergencyActive  = false;
};