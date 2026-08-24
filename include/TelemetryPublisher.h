#pragma once
#include <cstdint>
#include "TrinovaRobot.h"
#include "MqttClient.h"
#include "BatteryMonitor.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Publie périodiquement l'état du robot sur le topic telemetry.
class TelemetryPublisher {
public:
    TelemetryPublisher(TrinovaRobot& robot, SemaphoreHandle_t robotMutex,
                        MqttClient& mqtt, BatteryMonitor& battery,
                        const String& telemetryTopic);

    void publishOnce(); // à appeler périodiquement depuis TelemetryTask

private:
    const char* stateToString(RobotState state) const;

    TrinovaRobot&    _robot;
    SemaphoreHandle_t _robotMutex;
    MqttClient&      _mqtt;
    BatteryMonitor&  _battery;
    String           _telemetryTopic;
};