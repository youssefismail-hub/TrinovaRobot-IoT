#include "Pca9685Driver.h"
#include <Wire.h>

Pca9685Driver::Pca9685Driver(uint8_t i2cAddress, uint8_t sdaPin, uint8_t sclPin)
    : _pwm(i2cAddress), _address(i2cAddress), _sdaPin(sdaPin), _sclPin(sclPin) {}

bool Pca9685Driver::begin() {
    Wire.begin(_sdaPin, _sclPin);

    // Vérification ACK explicite AVANT toute commande — détecte l'absence du composant
    // dès begin(), pas seulement au premier moveServo() (exigence cahier des charges §11).
    Wire.beginTransmission(_address);
    if (Wire.endTransmission() != 0) {
        _ready = false;
        return false;
    }

    _pwm.begin();
    _pwm.setPWMFreq(50); // 50Hz standard pour servomoteurs
    _ready = true;
    return true;
}

void Pca9685Driver::setPwm(uint8_t channel, uint16_t onCount, uint16_t offCount) {
    if (!_ready) return;
    _pwm.setPWM(channel, onCount, offCount);
}

void Pca9685Driver::setFrequency(float freqHz) {
    if (!_ready) return;
    _pwm.setPWMFreq(freqHz);
}