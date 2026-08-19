#pragma once
#include <cstdint>
#include <Adafruit_PWMServoDriver.h>

// Pilote bas niveau I2C du PCA9685 — adaptation de la bibliothèque Adafruit.
class Pca9685Driver {
public:
    explicit Pca9685Driver(uint8_t i2cAddress = 0x40, uint8_t sdaPin = 8, uint8_t sclPin = 9);

    bool begin();               // false si le PCA9685 ne répond pas (ACK manquant)
    void setPwm(uint8_t channel, uint16_t onCount, uint16_t offCount);
    void setFrequency(float freqHz);
    bool isReady() const { return _ready; }

private:
    Adafruit_PWMServoDriver _pwm;
    uint8_t _address;
    uint8_t _sdaPin;
    uint8_t _sclPin;
    bool    _ready = false;
};