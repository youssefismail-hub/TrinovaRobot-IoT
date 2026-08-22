#include <Arduino.h>
#include "TrinovaRobot.h"
#include "WatchdogService.h"
#include "BatteryMonitor.h"
#include "DiagnosticsService.h"
#include "DeviceIdentity.h"
#include "WiFiManager.h"
#include "MqttClient.h"
#include "IoTConfig.h"
#include "Logger.h"

constexpr uint8_t  BATTERY_ADC_PIN     = 1;
constexpr float    BATTERY_DIVIDER     = 2.0f;
constexpr float    BATTERY_LOW_V       = 6.6f;
constexpr float    BATTERY_CRITICAL_V  = 6.0f;
constexpr uint32_t WATCHDOG_TIMEOUT_S  = 5;

TrinovaRobot   robot;
BatteryMonitor battery(BATTERY_ADC_PIN, BATTERY_DIVIDER, BATTERY_LOW_V, BATTERY_CRITICAL_V);
WiFiManager    wifiManager(IoTConfig::WIFI_SSID, IoTConfig::WIFI_PASSWORD);
MqttClient*    mqttClient = nullptr;

SemaphoreHandle_t g_robotMutex;
String g_statusTopic;

void motionTaskFn(void* /*pvParameters*/) {
    WatchdogService::registerCurrentTask();
    for (;;) {
        if (xSemaphoreTake(g_robotMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            robot.update();
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
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// --- IoTTask : Core 0, priorité basse, isolée du Core moteur/sécurité ---
void iotTaskFn(void* /*pvParameters*/) {
    WatchdogService::registerCurrentTask();
    wifiManager.begin();
    mqttClient->begin();

    for (;;) {
        wifiManager.loop();

        if (wifiManager.state() == WiFiConnectionState::Connected) {
            mqttClient->loop();
        }

        WatchdogService::feed();
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    DiagnosticsService::begin();
    DeviceIdentity::begin();
    battery.begin();

    g_statusTopic = "trinova/robot/" + DeviceIdentity::deviceId() + "/status";
    mqttClient = new MqttClient(IoTConfig::MQTT_BROKER_HOST, IoTConfig::MQTT_BROKER_PORT,
                                 DeviceIdentity::deviceId().c_str());
    mqttClient->setStatusTopic(g_statusTopic.c_str());

    bool ok = robot.begin(RobotMode::Mock);
    Serial.printf("[Setup] begin() = %s\n", ok ? "OK" : "FAIL");

    g_robotMutex = xSemaphoreCreateMutex();
    WatchdogService::begin(WATCHDOG_TIMEOUT_S);

    xTaskCreatePinnedToCore(motionTaskFn, "MotionTask", 4096, nullptr, 2, nullptr, 1);
    xTaskCreatePinnedToCore(safetyTaskFn, "SafetyTask", 4096, nullptr, 3, nullptr, 1);
    xTaskCreatePinnedToCore(iotTaskFn,    "IoTTask",    8192, nullptr, 1, nullptr, 0); // Core 0
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}