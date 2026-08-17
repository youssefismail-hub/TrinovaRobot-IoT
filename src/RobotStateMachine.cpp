#include "RobotStateMachine.h"

void RobotStateMachine::toIdle() {
    if (_state == RobotState::EmergencyStop) return; // seul clearEmergencyStop() peut sortir de cet état
    _state = RobotState::Idle;
}

void RobotStateMachine::toMoving() {
    if (_state == RobotState::EmergencyStop) return;
    _state = RobotState::Moving;
}

void RobotStateMachine::toEmergencyStop() {
    _state = RobotState::EmergencyStop; // priorité absolue, aucune garde
}

void RobotStateMachine::toError() {
    _state = RobotState::Error;
}

bool RobotStateMachine::clearEmergencyStop() {
    if (_state != RobotState::EmergencyStop) return false;
    _state = RobotState::Idle;
    return true;
}