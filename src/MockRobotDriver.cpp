#include "MockRobotDriver.h"
#include <Arduino.h>

bool MockRobotDriver::begin() {
    Serial.println("[Mock] begin()");
    _emergencyActive = false;
    return true;
}

void MockRobotDriver::setWheelCommand(WheelCommand cmd, uint8_t speed) {
    if (_emergencyActive) {
        Serial.println("[Mock] commande ignorée: emergency stop actif");
        return;
    }
    _lastCmd   = cmd;
    _lastSpeed = speed;

    const char* label = "STOP";
    switch (cmd) {
        case WheelCommand::Forward:  label = "FORWARD";   break;
        case WheelCommand::Backward: label = "BACKWARD";  break;
        case WheelCommand::TurnLeft: label = "TURN_LEFT";  break;
        case WheelCommand::TurnRight:label = "TURN_RIGHT"; break;
        case WheelCommand::Stop:     label = "STOP";      break;
    }
    Serial.printf("[Mock] %s speed=%u\n", label, speed);
}

void MockRobotDriver::stopWheels() {
    setWheelCommand(WheelCommand::Stop, 0);
}

bool MockRobotDriver::setServoAngle(uint8_t channel, uint8_t angle) {
    if (_emergencyActive) return false;
    Serial.printf("[Mock] Servo ch=%u angle=%u\n", channel, angle);
    return true;
}

void MockRobotDriver::emergencyStop() {
    _emergencyActive = true;
    _lastCmd = WheelCommand::Stop;
    _lastSpeed = 0;
    Serial.println("[Mock] EMERGENCY STOP");
}