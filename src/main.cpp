#include <Arduino.h>
#include "TrinovaRobot.h"
#include "WatchdogService.h"
#include "BatteryMonitor.h"
#include "DiagnosticsService.h"
#include "Logger.h"

// --- Config batterie (A AJUSTER selon ton pont diviseur réel) ---
constexpr uint8_t  BATTERY_ADC_PIN     = 1;
constexpr float    BATTERY_DIVIDER     = 2.0f;   // ex: pont 1:1 -> facteur 2 pour retrouver la tension réelle
constexpr float    BATTERY_LOW_V       = 6.6f;
constexpr float    BATTERY_CRITICAL_V  = 6.0f;

constexpr uint32_t WATCHDOG_TIMEOUT_S  = 5;

TrinovaRobot   robot;
BatteryMonitor battery(BATTERY_ADC_PIN, BATTERY_DIVIDER, BATTERY_LOW_V, BATTERY_CRITICAL_V);

// Mutex protégeant l'accès concurrent à `robot` depuis plusieurs tâches.
SemaphoreHandle_t g_robotMutex;

// --- MotionTask : exécute le cycle de mouvement + update() (timeout) ---
void motionTaskFn(void* /*pvParameters*/) {
    WatchdogService::registerCurrentTask();

    for (;;) {
        if (xSemaphoreTake(g_robotMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            robot.update(); // vérifie le timeout de commande (MotionSafety)

            robot.forward(40);
            xSemaphoreGive(g_robotMutex);
            vTaskDelay(pdMS_TO_TICKS(800));

            xSemaphoreTake(g_robotMutex, portMAX_DELAY);
            robot.turnLeft(30);
            xSemaphoreGive(g_robotMutex);
            vTaskDelay(pdMS_TO_TICKS(400));

            xSemaphoreTake(g_robotMutex, portMAX_DELAY);
            robot.stop();
            xSemaphoreGive(g_robotMutex);
            vTaskDelay(pdMS_TO_TICKS(1500));
        }

        WatchdogService::feed();
    }
}

// --- SafetyTask : priorité plus haute, surveille batterie + heap, feed watchdog indépendamment ---
void safetyTaskFn(void* /*pvParameters*/) {
    WatchdogService::registerCurrentTask();

    for (;;) {
        float voltage = battery.readVoltage();
        BatteryLevel lvl = battery.level();

        if (lvl == BatteryLevel::Critical) {
            Logger::log(LogLevel::Error, "Safety", "Batterie critique -> emergencyStop()");
            if (xSemaphoreTake(g_robotMutex, pdMS_TO_TICKS(200)) == pdTRUE) {
                robot.emergencyStop();
                xSemaphoreGive(g_robotMutex);
            }
            DiagnosticsService::incrementErrorCount();
        } else if (lvl == BatteryLevel::Low) {
            Logger::log(LogLevel::Warn, "Safety", "Batterie faible");
        }

        if (DiagnosticsService::freeHeap() < 20000) {
            Logger::log(LogLevel::Warn, "Diagnostics", "Heap bas");
        }

        WatchdogService::feed();
        vTaskDelay(pdMS_TO_TICKS(500)); // fréquence de surveillance : 2x/seconde
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    DiagnosticsService::begin();
    battery.begin();

    bool ok = robot.begin(RobotMode::Mock); // repasse en Hardware quand le banc est prêt
    Serial.printf("[Setup] begin() = %s\n", ok ? "OK" : "FAIL");

    g_robotMutex = xSemaphoreCreateMutex();

    WatchdogService::begin(WATCHDOG_TIMEOUT_S);

    // Core 1 = temps réel moteur/sécurité (recommandé sur ESP32 : Core 0 réservé au Wi-Fi/BT en interne)
    xTaskCreatePinnedToCore(motionTaskFn, "MotionTask", 4096, nullptr, 2, nullptr, 1);
    xTaskCreatePinnedToCore(safetyTaskFn, "SafetyTask", 4096, nullptr, 3, nullptr, 1); // priorité > MotionTask
}

void loop() {
    // Rien ici : tout tourne dans les tâches FreeRTOS dédiées.
    vTaskDelay(pdMS_TO_TICKS(1000));
}