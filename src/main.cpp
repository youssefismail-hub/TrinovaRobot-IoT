#include <Arduino.h>
#include <ArduinoJson.h>
#include "TrinovaRobot.h"
#include "WatchdogService.h"
#include "BatteryMonitor.h"
#include "DiagnosticsService.h"
#include "DeviceIdentity.h"
#include "WiFiManager.h"
#include "MqttClient.h"
#include "CommandProcessor.h"
#include "TelemetryPublisher.h"
#include "OtaManager.h"
#include "OfflineBuffer.h"
#include "Topics.h"
#include "IoTConfig.h"
#include "Logger.h"
#include "TimeService.h"
#include "MqttCertificates.h"

constexpr uint8_t  BATTERY_ADC_PIN     = 1;
constexpr float    BATTERY_DIVIDER     = 2.0f;
constexpr float    BATTERY_LOW_V       = 6.6f;
constexpr float    BATTERY_CRITICAL_V  = 6.0f;
constexpr uint32_t WATCHDOG_TIMEOUT_S  = 5;
constexpr uint8_t  OFFLINE_FLUSH_PER_CYCLE = 5; // évite de flooder le broker après une longue coupure

TrinovaRobot   robot;
BatteryMonitor battery(BATTERY_ADC_PIN, BATTERY_DIVIDER, BATTERY_LOW_V, BATTERY_CRITICAL_V);
WiFiManager    wifiManager(IoTConfig::WIFI_SSID, IoTConfig::WIFI_PASSWORD);
MqttClient*    mqttClient = nullptr;
CommandProcessor*   commandProcessor   = nullptr;
TelemetryPublisher* telemetryPublisher = nullptr;

SemaphoreHandle_t g_robotMutex;
String g_commandTopic, g_statusTopic, g_telemetryTopic, g_errorTopic, g_otaTopic;

void motionTaskFn(void*) {
    WatchdogService::registerCurrentTask();
    for (;;) {
        if (xSemaphoreTake(g_robotMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            robot.update();
            xSemaphoreGive(g_robotMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(200));
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
            // Toujours poussé dans le buffer : si MQTT est connecté, il sera drainé
            // au prochain cycle IoTTask ; sinon il attend la reconnexion.
            OfflineBuffer::push("ERROR", "Safety", "battery_critical_emergency_stop");
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
        mqttClient->subscribe(g_otaTopic.c_str());
        Logger::log(LogLevel::Info, "MQTT", "Souscrit command + ota");
    });

    mqttClient->setOnMessage([](const char* topic, const uint8_t* payload, unsigned int length) {
        if (g_commandTopic.equals(topic)) {
            commandProcessor->handleMessage(topic, payload, length);
            return;
        }
        if (g_otaTopic.equals(topic)) {
            StaticJsonDocument<192> doc;
            if (deserializeJson(doc, payload, length) != DeserializationError::Ok) {
                Logger::log(LogLevel::Warn, "OTA", "Payload OTA invalide");
                return;
            }
            const char* url = doc["url"] | "";
            if (strlen(url) == 0) {
                Logger::log(LogLevel::Warn, "OTA", "URL manquante");
                return;
            }
            // Note : bloquant volontairement — une mise à jour OTA ne doit PAS
            // se faire en tâche de fond pendant que le robot bouge. On accepte
            // le blocage d'IoTTask (Core 0), ça n'affecte pas MotionTask/SafetyTask (Core 1).
            OtaManager::performUpdate(String(url));
        }
    });

    for (;;) {
        wifiManager.loop();

        if (wifiManager.state() == WiFiConnectionState::Connected) {
            mqttClient->loop();

            // Draine le buffer offline si connecté, par petits lots pour ne pas saturer le broker.
            if (mqttClient->isConnected()) {
                uint8_t flushed = 0;
                OfflineEvent ev;
                while (flushed < OFFLINE_FLUSH_PER_CYCLE && OfflineBuffer::pop(ev)) {
                    StaticJsonDocument<192> doc;
                    doc["ts"]      = ev.timestampMs;
                    doc["level"]   = ev.level;
                    doc["tag"]     = ev.tag;
                    doc["message"] = ev.message;
                    char buf[192];
                    serializeJson(doc, buf);
                    mqttClient->publish(g_errorTopic.c_str(), buf);
                    flushed++;
                }
            }
        }

        WatchdogService::feed();
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

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
    OfflineBuffer::begin();
    battery.begin();

    g_statusTopic    = Topics::status(DeviceIdentity::deviceId());
    g_telemetryTopic = Topics::telemetry(DeviceIdentity::deviceId());
    g_commandTopic   = Topics::command(DeviceIdentity::deviceId());
    g_errorTopic     = Topics::error(DeviceIdentity::deviceId());
    g_otaTopic       = Topics::ota(DeviceIdentity::deviceId());

    Logger::log(LogLevel::Info, "Firmware", OtaManager::currentVersion());

   
    WiFi.begin(IoTConfig::WIFI_SSID, IoTConfig::WIFI_PASSWORD);
    uint32_t wifiStart = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < 15000) delay(200);
    if (WiFi.status() == WL_CONNECTED) {
        TimeService::syncNow();
    }

    mqttClient = new MqttClient(IoTConfig::MQTT_BROKER_HOST, IoTConfig::MQTT_BROKER_PORT,
                                 DeviceIdentity::deviceId().c_str(), MQTT_ROOT_CA); // NOUVEAU: rootCA
    mqttClient->setStatusTopic(g_statusTopic.c_str());

    commandProcessor = new CommandProcessor(robot, g_robotMutex, IoTConfig::COMMAND_HMAC_SECRET); // NOUVEAU
    




    
    

    bool ok = robot.begin(RobotMode::Mock);
    Serial.printf("[Setup] begin() = %s\n", ok ? "OK" : "FAIL");

    g_robotMutex = xSemaphoreCreateMutex();
    commandProcessor   = new CommandProcessor(robot, g_robotMutex);
    telemetryPublisher = new TelemetryPublisher(robot, g_robotMutex, *mqttClient, battery, g_telemetryTopic);

    WatchdogService::begin(WATCHDOG_TIMEOUT_S);

    xTaskCreatePinnedToCore(motionTaskFn,    "MotionTask",    4096, nullptr, 2, nullptr, 1);
    xTaskCreatePinnedToCore(safetyTaskFn,    "SafetyTask",    4096, nullptr, 3, nullptr, 1);
    xTaskCreatePinnedToCore(iotTaskFn,       "IoTTask",       8192, nullptr, 1, nullptr, 0);
    xTaskCreatePinnedToCore(telemetryTaskFn, "TelemetryTask", 4096, nullptr, 1, nullptr, 0);
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}