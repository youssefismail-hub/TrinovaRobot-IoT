#pragma once
#include <Arduino.h>

// Mise à jour firmware déclenchée à distance (via MQTT).
// ⚠️ HTTP simple pour le développement/tests. En production : WiFiClientSecure + certificat épinglé.
class OtaManager {
public:
    static void performUpdate(const String& url);
    static const char* currentVersion();
};