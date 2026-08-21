#include "DiagnosticsService.h"
#include <Arduino.h>
#include <esp_system.h>
#include "Logger.h"

uint32_t DiagnosticsService::_errorCount = 0;

void DiagnosticsService::begin() {
    logBootReason();
    _errorCount = 0;
}

void DiagnosticsService::logBootReason() {
    esp_reset_reason_t reason = esp_reset_reason();
    const char* label = "UNKNOWN";
    switch (reason) {
        case ESP_RST_POWERON:  label = "POWERON";        break;
        case ESP_RST_SW:       label = "SOFTWARE_RESET";  break;
        case ESP_RST_PANIC:    label = "PANIC";           break;
        case ESP_RST_INT_WDT:  label = "INTERRUPT_WDT";   break;
        case ESP_RST_TASK_WDT: label = "TASK_WDT";        break;
        case ESP_RST_WDT:      label = "OTHER_WDT";       break;
        case ESP_RST_BROWNOUT: label = "BROWNOUT";        break;
        default: break;
    }
    Logger::log(LogLevel::Info, "Diagnostics", label);
}

uint32_t DiagnosticsService::freeHeap() {
    return ESP.getFreeHeap();
}

void DiagnosticsService::incrementErrorCount() {
    _errorCount++;
}

uint32_t DiagnosticsService::errorCount() {
    return _errorCount;
}