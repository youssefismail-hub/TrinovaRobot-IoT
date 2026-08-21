#include "BatteryMonitor.h"
#include <Arduino.h>

BatteryMonitor::BatteryMonitor(uint8_t adcPin, float dividerRatio, float lowVoltage, float criticalVoltage)
    : _adcPin(adcPin), _dividerRatio(dividerRatio), _lowVoltage(lowVoltage), _criticalVoltage(criticalVoltage) {}

void BatteryMonitor::begin() {
    pinMode(_adcPin, INPUT);
}

float BatteryMonitor::readVoltage() {
    int raw = analogRead(_adcPin);          // 0-4095 sur ESP32-S3 (ADC 12 bits)
    float adcVoltage = (raw / 4095.0f) * 3.3f;
    _lastVoltage = adcVoltage * _dividerRatio;
    return _lastVoltage;
}

BatteryLevel BatteryMonitor::level() {
    if (_lastVoltage <= _criticalVoltage) return BatteryLevel::Critical;
    if (_lastVoltage <= _lowVoltage)      return BatteryLevel::Low;
    return BatteryLevel::Normal;
}