#include "DeviceIdentity.h"
#include <Preferences.h>
#include "Logger.h"

String DeviceIdentity::_deviceId = "";

void DeviceIdentity::begin() {
    Preferences prefs;
    prefs.begin("trinova", false); 

    String stored = prefs.getString("device_id", "");
    if (stored.length() > 0) {
        _deviceId = stored;
    } else {
        uint64_t mac = ESP.getEfuseMac();
        char buf[32];
        snprintf(buf, sizeof(buf), "robot-ESP32-%04X%08X",
                 (uint16_t)(mac >> 32), (uint32_t)mac);
        _deviceId = String(buf);
        prefs.putString("device_id", _deviceId);
    }

    prefs.end();
    Logger::log(LogLevel::Info, "Identity", _deviceId.c_str());
}

const String& DeviceIdentity::deviceId() {
    return _deviceId;
}