#pragma once
#include <cstdint>
#include "RobotCommand.h"
#include "TrinovaRobot.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class CommandProcessor {
public:
    CommandProcessor(TrinovaRobot& robot, SemaphoreHandle_t robotMutex, const char* hmacSecret);

    void handleMessage(const char* topic, const uint8_t* payload, unsigned int length);

    static RobotCommand parse(const uint8_t* payload, unsigned int length);

private:
    bool verifySignature(const uint8_t* payload, unsigned int length) const;
    void execute(const RobotCommand& cmd);

    TrinovaRobot&      _robot;
    SemaphoreHandle_t  _robotMutex;
    const char*        _hmacSecret;
};