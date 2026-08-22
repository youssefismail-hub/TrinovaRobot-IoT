#pragma once
#include <cstdint>

enum class WiFiConnectionState { Disconnected, Connecting, Connected };

// Connexion Wi-Fi non bloquante avec backoff exponentiel. À appeler dans
// une tâche dédiée (IoTTask) — ne bloque jamais le thread appelant.
class WiFiManager {
public:
    WiFiManager(const char* ssid, const char* password);

    void begin();
    void loop(); // à appeler périodiquement depuis IoTTask
    WiFiConnectionState state() const { return _state; }
    int8_t rssi() const;

private:
    void attemptConnect(uint32_t nowMs);

    const char* _ssid;
    const char* _password;
    WiFiConnectionState _state = WiFiConnectionState::Disconnected;
    uint32_t _lastAttemptMs = 0;
    uint32_t _backoffMs = 1000;
};