#pragma once
#include "RobotTypes.h"

// Transitions d'état pures — aucune dépendance matérielle, 100% testable en natif.
class RobotStateMachine {
public:
    RobotState current() const { return _state; }

    void toIdle();
    void toMoving();
    void toEmergencyStop();
    void toError();
    bool clearEmergencyStop(); // false si pas en EmergencyStop

private:
    RobotState _state = RobotState::Idle;
};