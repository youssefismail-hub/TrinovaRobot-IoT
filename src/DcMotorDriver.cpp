#include "DcMotorDriver.h"
#include <Arduino.h>
#include "HardwarePins.h"

DcMotorDriver::DcMotorDriver(const MotorPins& pins, uint16_t pwmFrequency)
    : _pins(pins), _pwmFrequency(pwmFrequency) {}

bool DcMotorDriver::begin() {
    pinMode(_pins.leftIn1, OUTPUT);
    pinMode(_pins.leftIn2, OUTPUT);
    pinMode(_pins.rightIn1, OUTPUT);
    pinMode(_pins.rightIn2, OUTPUT);

    // Ancienne API LEDC (core Arduino-ESP32 < 3.0) : setup par canal, puis attach du pin au canal.
    ledcSetup(HardwareConfig::LEDC_CHANNEL_LEFT, _pwmFrequency, 8);
    ledcAttachPin(_pins.leftPwm, HardwareConfig::LEDC_CHANNEL_LEFT);

    ledcSetup(HardwareConfig::LEDC_CHANNEL_RIGHT, _pwmFrequency, 8);
    ledcAttachPin(_pins.rightPwm, HardwareConfig::LEDC_CHANNEL_RIGHT);

    stopAll();
    _initialized = true;
    return true;
}

uint8_t DcMotorDriver::speedToDuty(uint8_t speed) const {
    if (speed > 100) speed = 100;
    return static_cast<uint8_t>((speed * 255) / 100);
}

void DcMotorDriver::applyMotor(uint8_t in1, uint8_t in2, uint8_t ledcChannel, Direction dir, uint8_t speed) {
    switch (dir) {
        case Direction::Forward:
            digitalWrite(in1, HIGH);
            digitalWrite(in2, LOW);
            break;
        case Direction::Backward:
            digitalWrite(in1, LOW);
            digitalWrite(in2, HIGH);
            break;
        case Direction::Stop:
            digitalWrite(in1, LOW);
            digitalWrite(in2, LOW);
            break;
    }
    ledcWrite(ledcChannel, dir == Direction::Stop ? 0 : speedToDuty(speed));
}

void DcMotorDriver::setLeftMotor(Direction dir, uint8_t speed) {
    if (!_initialized) return;
    applyMotor(_pins.leftIn1, _pins.leftIn2, HardwareConfig::LEDC_CHANNEL_LEFT, dir, speed);
}

void DcMotorDriver::setRightMotor(Direction dir, uint8_t speed) {
    if (!_initialized) return;
    applyMotor(_pins.rightIn1, _pins.rightIn2, HardwareConfig::LEDC_CHANNEL_RIGHT, dir, speed);
}

void DcMotorDriver::stopAll() {
    digitalWrite(_pins.leftIn1, LOW);
    digitalWrite(_pins.leftIn2, LOW);
    digitalWrite(_pins.rightIn1, LOW);
    digitalWrite(_pins.rightIn2, LOW);
    ledcWrite(HardwareConfig::LEDC_CHANNEL_LEFT, 0);
    ledcWrite(HardwareConfig::LEDC_CHANNEL_RIGHT, 0);
}