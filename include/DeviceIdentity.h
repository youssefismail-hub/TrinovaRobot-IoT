#pragma once
#include <Arduino.h>

// Génère et persiste un identifiant unique par appareil : robot-ESP32-XXXXXX

class DeviceIdentity {
public:
    static void begin();
    static const String& deviceId();

private:
    static String _deviceId;
};