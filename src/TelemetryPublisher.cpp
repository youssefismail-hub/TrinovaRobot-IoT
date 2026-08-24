#include "TelemetryPublisher.h"
#include <ArduinoJson.h>
#include <Arduino.h>
#include <WiFi.h>
#include "DiagnosticsService.h"

TelemetryPublisher::TelemetryPublisher(TrinovaRobot& robot, SemaphoreHandle_t robotMutex,
                                        MqttClient& mqtt, BatteryMonitor& battery,
                                        const String& telemetryTopic)
    : _robot(robot), _robotMutex(robotMutex), _mqtt(mqtt), _battery(battery),
      _telemetryTopic(telemetryTopic) {}

const char* TelemetryPublisher::stateToString(RobotState state) const {
    switch (state) {
        case RobotState::Idle:           return "IDLE";
        case RobotState::Moving:         return "MOVING";
        case RobotState::EmergencyStop:  return "EMERGENCY_STOP";
        case RobotState::Error:          return "ERROR";
    }
    return "UNKNOWN";
}

void TelemetryPublisher::publishOnce() {
    if (!_mqtt.isConnected()) return;

    RobotState state;
    if (xSemaphoreTake(_robotMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        state = _robot.state();
        xSemaphoreGive(_robotMutex);
    } else {
        return; // on republiera au prochain cycle plutôt que de bloquer
    }

    StaticJsonDocument<256> doc;
    doc["state"]   = stateToString(state);
    doc["battery"] = _battery.readVoltage();
    doc["heap"]    = DiagnosticsService::freeHeap();
    doc["rssi"]    = WiFi.RSSI();
    doc["uptime"]  = millis() / 1000;
    doc["errors"]  = DiagnosticsService::errorCount();

    char buffer[256];
    size_t len = serializeJson(doc, buffer);
    _mqtt.publish(_telemetryTopic.c_str(), buffer);
}