#pragma once
#include <cstdint>

enum class RobotMode {
    Mock,
    Hardware
};

enum class RobotState {
    Idle,
    Moving,
    EmergencyStop,
    Error
};

enum class TrinovaError {
    None,
    NotInitialized,
    InvalidSpeed,
    InvalidAngle,
    MotorDriverError,
    ServoDriverError,
    EmergencyStopActive
};

struct RobotConfig {
    uint8_t  defaultSpeed     = 40;
    uint32_t commandTimeoutMs = 1500;
    uint16_t pwmFrequency     = 20000;
    uint8_t  minAngle         = 0;
    uint8_t  maxAngle         = 180;
};