#pragma once
#include <cstdint>
#include "Pca9685Driver.h"

// Convertit un angle 0-180° en signal PWM PCA9685, avec bornes par voie.
class ServoDriver {
public:
    explicit ServoDriver(Pca9685Driver& pca);

    bool begin();
    bool setAngle(uint8_t channel, uint8_t angle); // angle déjà borné en amont par MotionSafety

private:
    uint16_t angleToPulse(uint8_t angle) const;

    Pca9685Driver& _pca;
    // Valeurs typiques pour SG90 à 50Hz sur registre 12 bits (0-4095) — ajustable si besoin.
    static constexpr uint16_t PULSE_MIN = 102;  // ~0°
    static constexpr uint16_t PULSE_MAX = 512;  // ~180°
};