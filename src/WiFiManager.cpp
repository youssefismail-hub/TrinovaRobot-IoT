#include "WiFiManager.h"
#include <WiFi.h>
#include <Arduino.h>
#include "Logger.h"
#include "IoTConfig.h"

WiFiManager::WiFiManager(const char* ssid, const char* password)
    : _ssid(ssid), _password(password) {}

void WiFiManager::begin() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    _backoffMs = IoTConfig::WIFI_RECONNECT_MIN_MS;
}

void WiFiManager::attemptConnect(uint32_t nowMs) {
    Logger::log(LogLevel::Info, "WiFi", "Tentative de connexion...");
    WiFi.begin(_ssid, _password);
    _state = WiFiConnectionState::Connecting;
    _lastAttemptMs = nowMs;
}

void WiFiManager::loop() {
    uint32_t now = millis();

    if (WiFi.status() == WL_CONNECTED) {
        if (_state != WiFiConnectionState::Connected) {
            Logger::log(LogLevel::Info, "WiFi", "Connecte");
            _state = WiFiConnectionState::Connected;
            _backoffMs = IoTConfig::WIFI_RECONNECT_MIN_MS; // reset backoff sur succès
        }
        return;
    }

    // Déconnecté ou en cours de connexion échouée
    if (_state == WiFiConnectionState::Connected) {
        Logger::log(LogLevel::Warn, "WiFi", "Connexion perdue");
    }
    _state = WiFiConnectionState::Disconnected;

    if (now - _lastAttemptMs >= _backoffMs) {
        attemptConnect(now);
        // Backoff exponentiel borné (évite de spammer le point d'accès)
        _backoffMs = min(_backoffMs * 2, IoTConfig::WIFI_RECONNECT_MAX_MS);
    }
}

int8_t WiFiManager::rssi() const {
    return WiFi.RSSI();
}