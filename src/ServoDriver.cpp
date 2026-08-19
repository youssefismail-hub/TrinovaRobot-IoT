#include "ServoDriver.h"

ServoDriver::ServoDriver(Pca9685Driver& pca) : _pca(pca) {}

bool ServoDriver::begin() {
    return _pca.isReady();
}

uint16_t ServoDriver::angleToPulse(uint8_t angle) const {
    if (angle > 180) angle = 180;
    return PULSE_MIN + ((uint32_t)(PULSE_MAX - PULSE_MIN) * angle) / 180;
}

bool ServoDriver::setAngle(uint8_t channel, uint8_t angle) {
    if (!_pca.isReady()) return false;
    uint16_t pulse = angleToPulse(angle);
    _pca.setPwm(channel, 0, pulse);
    return true;
}