#pragma once
#include <cstdint>

enum class CommandType {
    Forward,
    Backward,
    TurnLeft,
    TurnRight,
    Stop,
    Servo,
    EmergencyStop,
    ClearEmergencyStop,
    Unknown
};

struct RobotCommand {
    CommandType type = CommandType::Unknown;
    uint8_t speed        = 0;   
    uint8_t servoChannel  = 0;
    uint8_t servoAngle    = 0;  
};