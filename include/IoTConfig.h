#pragma once


#include <cstdint>

// Config réseau — A REMPLACER avec tes identifiants .


namespace IoTConfig {
    constexpr const char* WIFI_SSID     = "TON_SSID";
    constexpr const char* WIFI_PASSWORD = "TON_MOT_DE_PASSE";

    constexpr const char* MQTT_BROKER_HOST = "test.mosquitto.org"; // broker public pour les tests
    constexpr uint16_t    MQTT_BROKER_PORT = 1883;                  // 8883 pour TLS

    constexpr uint32_t WIFI_RECONNECT_MIN_MS = 1000u;
    constexpr uint32_t WIFI_RECONNECT_MAX_MS = 30000u;
}