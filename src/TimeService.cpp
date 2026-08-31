#include "TimeService.h"
#include <Arduino.h>
#include <time.h>
#include "Logger.h"

bool TimeService::syncNow(uint32_t timeoutMs) {
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    uint32_t start = millis();
    time_t now = time(nullptr);
    while (now < 100000 && (millis() - start) < timeoutMs) { // 100000 = date absurde -> pas encore synchro
        delay(200);
        now = time(nullptr);
    }

    bool ok = now >= 100000;
    Logger::log(ok ? LogLevel::Info : LogLevel::Error, "Time", ok ? "NTP synchronise" : "NTP timeout");
    return ok;
}