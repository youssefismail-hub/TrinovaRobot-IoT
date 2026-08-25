#pragma once
#include <Arduino.h>

namespace Topics {
    inline String status(const String& deviceId)    { return "trinova/robot/" + deviceId + "/status"; }
    inline String telemetry(const String& deviceId)  { return "trinova/robot/" + deviceId + "/telemetry"; }
    inline String command(const String& deviceId)    { return "trinova/robot/" + deviceId + "/command"; }
    inline String error(const String& deviceId)      { return "trinova/robot/" + deviceId + "/error"; }
    inline String ota(const String& deviceId)        { return "trinova/robot/" + deviceId + "/ota"; }
}