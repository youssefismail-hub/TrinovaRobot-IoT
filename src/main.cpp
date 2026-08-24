#include <Arduino.h>
#include "TrinovaRobot.h"
#include "WatchdogService.h"
#include "BatteryMonitor.h"
#include "DiagnosticsService.h"
#include "DeviceIdentity.h"
#include "WiFiManager.h"
#include "MqttClient.h"
#include "CommandProcessor.h"
#include "TelemetryPublisher.h"
#include "Topics.h"
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
CommandProcessor*   commandProcessor   = nullptr;
TelemetryPublisher* telemetryPublisher = nullptr;

SemaphoreHandle_t g_robotMutex;
String g_commandTopic;
String g_statusTopic;
String g_telemetryTopic;

void motionTaskFn(void*) {
    WatchdogService::registerCurrentTask();
    for (;;) {
        if (xSemaphoreTake(g_robotMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            robot.update();
            xSemaphoreGive(g_robotMutex);
        }
        // Le mouvement automatique de démo peut rester ou être retiré maintenant
        // que le pilotage se fait par MQTT — laissé ici pour continuer à voir le robot bouger.
        if (xSemaphoreTake(g_robotMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            robot.forward(40);
            xSemaphoreGive(g_robotMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(800));

        if (xSemaphoreTake(g_robotMutex, portMAX_DELAY) == pdTRUE) {
            robot.stop();
            xSemaphoreGive(g_robotMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(1500));

        WatchdogService::feed();
    }
}

void safetyTaskFn(void*) {
    WatchdogService::registerCurrentTask();
    for (;;) {
        BatteryLevel lvl = battery.level();
        if (lvl == BatteryLevel::Critical) {
            Logger::log(LogLevel::Error, "Safety", "Batterie critique -> emergencyStop()");
            if (xSemaphoreTake(g_robotMutex, pdMS_TO_TICKS(200)) == pdTRUE) {
                robot.emergencyStop();
                xSemaphoreGive(g_robotMutex);
            }
            DiagnosticsService::incrementErrorCount();
        }
        battery.readVoltage();
        WatchdogService::feed();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void iotTaskFn(void*) {
    WatchdogService::registerCurrentTask();
    wifiManager.begin();
    mqttClient->begin();

    mqttClient->setOnConnected([]() {
        mqttClient->subscribe(g_commandTopic.c_str());
        Logger::log(LogLevel::Info, "MQTT", "Souscrit au topic de commande");
    });

    mqttClient->setOnMessage([](const char* topic, const uint8_t* payload, unsigned int length) {
        if (g_commandTopic.equals(topic)) {
            commandProcessor->handleMessage(topic, payload, length);
        }
    });

    for (;;) {
        wifiManager.loop();
        if (wifiManager.state() == WiFiConnectionState::Connected) {
            mqttClient->loop();
        }
        WatchdogService::feed();
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

// --- TelemetryTask : Core 0, publie l'état toutes les secondes ---
void telemetryTaskFn(void*) {
    WatchdogService::registerCurrentTask();
    for (;;) {
        telemetryPublisher->publishOnce();
        WatchdogService::feed();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    DiagnosticsService::begin();
    DeviceIdentity::begin();
    battery.begin();

    g_statusTopic    = Topics::status(DeviceIdentity::deviceId());
    g_telemetryTopic = Topics::telemetry(DeviceIdentity::deviceId());
    g_commandTopic   = Topics::command(DeviceIdentity::deviceId());

    mqttClient = new MqttClient(IoTConfig::MQTT_BROKER_HOST, IoTConfig::MQTT_BROKER_PORT,
                                 DeviceIdentity::deviceId().c_str());
    mqttClient->setStatusTopic(g_statusTopic.c_str());

    bool ok = robot.begin(RobotMode::Mock);
    Serial.printf("[Setup] begin() = %s\n", ok ? "OK" : "FAIL");

    g_robotMutex = xSemaphoreCreateMutex();

    commandProcessor   = new CommandProcessor(robot, g_robotMutex);
    telemetryPublisher = new TelemetryPublisher(robot, g_robotMutex, *mqttClient, battery, g_telemetryTopic);

    WatchdogService::begin(WATCHDOG_TIMEOUT_S);

    xTaskCreatePinnedToCore(motionTaskFn,    "MotionTask",    4096, nullptr, 2, nullptr, 1);
    xTaskCreatePinnedToCore(safetyTaskFn,    "SafetyTask",    4096, nullptr, 3, nullptr, 1);

    xTaskCreatePinnedToCore(motionTaskFn,    "MotionTask",    4096, nullptr, 2, nullptr, 1);
    xTaskCreatePinnedToCore(safetyTaskFn,    "SafetyTask",    4096, nullptr, 3, nullptr, 1);
    xTaskCreatePinnedToCore(iotTaskFn,       "IoTTask",       8192, nullptr, 1, nullptr, 0);
    xTaskCreatePinnedToCore(telemetryTaskFn, "TelemetryTask", 4096, nullptr, 1, nullptr, 0);
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}