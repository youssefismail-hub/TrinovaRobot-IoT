#pragma once
#include <Arduino.h>

// Centralise la construction des topics MQTT — une seule source de vérité.
namespace Topics {
    inline String status(const String& deviceId)    { return "trinova/robot/" + deviceId + "/status"; }
    inline String telemetry(const String& deviceId)  { return "trinova/robot/" + deviceId + "/telemetry"; }
    inline String command(const String& deviceId)    { return "trinova/robot/" + deviceId + "/command"; }
    inline String error(const String& deviceId)      { return "trinova/robot/" + deviceId + "/error"; }
}