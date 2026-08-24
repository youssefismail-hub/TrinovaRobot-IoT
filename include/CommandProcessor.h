#pragma once
#include <cstdint>
#include "RobotCommand.h"
#include "TrinovaRobot.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Parse le JSON reçu par MQTT en RobotCommand, puis l'exécute sur TrinovaRobot.

class CommandProcessor {
public:
    CommandProcessor(TrinovaRobot& robot, SemaphoreHandle_t robotMutex);

    // Appelé directement par le callback MQTT (thread IoTTask).
    void handleMessage(const char* topic, const uint8_t* payload, unsigned int length);

    static RobotCommand parse(const uint8_t* payload, unsigned int length);

private:
    void execute(const RobotCommand& cmd);

    TrinovaRobot&      _robot;
    SemaphoreHandle_t  _robotMutex;
};