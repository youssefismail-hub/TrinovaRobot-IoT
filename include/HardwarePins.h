#pragma once
#include <cstdint>

struct MotorPins {
    uint8_t leftIn1;
    uint8_t leftIn2;
    uint8_t leftPwm;
    uint8_t rightIn1;
    uint8_t rightIn2;
    uint8_t rightPwm;
};

namespace HardwareConfig {
    constexpr MotorPins DefaultMotorPins = {
        /* leftIn1  */ 4,
        /* leftIn2  */ 5,
        /* leftPwm  */ 6,
        /* rightIn1 */ 7,
        /* rightIn2 */ 15,
        /* rightPwm */ 16
    };

    // Renommé (PCA9685_I2C_ADDRESS est une macro definie par Adafruit_PWMServoDriver.h)
    constexpr uint8_t DEFAULT_PCA9685_ADDRESS = 0x40;
    constexpr uint8_t I2C_SDA_PIN = 8;
    constexpr uint8_t I2C_SCL_PIN = 9;

    // Canaux LEDC dédiés aux moteurs (0-15 disponibles sur ESP32-S3)
    constexpr uint8_t LEDC_CHANNEL_LEFT  = 0;
    constexpr uint8_t LEDC_CHANNEL_RIGHT = 1;
}