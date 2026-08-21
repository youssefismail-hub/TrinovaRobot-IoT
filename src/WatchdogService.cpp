#include "WatchdogService.h"
#include <esp_task_wdt.h>
#include "Logger.h"

bool WatchdogService::begin(uint32_t timeoutSeconds) {
    esp_err_t err = esp_task_wdt_init(timeoutSeconds, true); // panic=true -> reset si non feed
    if (err != ESP_OK) {
        Logger::log(LogLevel::Error, "Watchdog", "esp_task_wdt_init failed");
        return false;
    }
    Logger::log(LogLevel::Info, "Watchdog", "initialized");
    return true;
}

void WatchdogService::registerCurrentTask() {
    esp_task_wdt_add(NULL); // NULL = tâche appelante
}

void WatchdogService::feed() {
    esp_task_wdt_reset();
}