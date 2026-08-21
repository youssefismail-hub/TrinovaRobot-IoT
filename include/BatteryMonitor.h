#pragma once
#include <cstdint>

enum class BatteryLevel { Normal, Low, Critical };

// Lecture ADC + seuils. Le ratio du pont diviseur dépend de ton montage réel —
// à calibrer avec un multimètre avant de faire confiance aux valeurs.
class BatteryMonitor {
public:
    BatteryMonitor(uint8_t adcPin, float dividerRatio, float lowVoltage, float criticalVoltage);

    void begin();
    float readVoltage();
    BatteryLevel level();

private:
    uint8_t _adcPin;
    float   _dividerRatio;
    float   _lowVoltage;
    float   _criticalVoltage;
    float   _lastVoltage = 0.0f;
};