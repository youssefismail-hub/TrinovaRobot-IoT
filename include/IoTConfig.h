#pragma once


#include <cstdint>



namespace IoTConfig {
    constexpr const char* WIFI_SSID     = "TON_SSID";
    constexpr const char* WIFI_PASSWORD = "TON_MOT_DE_PASSE";

    constexpr const char* MQTT_BROKER_HOST = "test.mosquitto.org";
    constexpr uint16_t    MQTT_BROKER_PORT = 8883; 

    constexpr uint32_t WIFI_RECONNECT_MIN_MS = 1000;
    constexpr uint32_t WIFI_RECONNECT_MAX_MS = 30000;

    
   
    constexpr const char* COMMAND_HMAC_SECRET = "REMPLACE_PAR_UN_SECRET_GENERE_ALEATOIREMENT";
}