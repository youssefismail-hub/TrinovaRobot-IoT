#include "CommandProcessor.h"
#include <ArduinoJson.h>
#include "Logger.h"

CommandProcessor::CommandProcessor(TrinovaRobot& robot, SemaphoreHandle_t robotMutex)
    : _robot(robot), _robotMutex(robotMutex) {}

RobotCommand CommandProcessor::parse(const uint8_t* payload, unsigned int length) {
    RobotCommand cmd;

    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, payload, length);
    if (err) {
        Logger::log(LogLevel::Warn, "CommandProcessor", "JSON invalide");
        return cmd; 
    }

    const char* type = doc["type"] | "";

    if (strcmp(type, "move") == 0) {
        const char* direction = doc["direction"] | "";
        cmd.speed = doc["speed"] | 0; 
        if      (strcmp(direction, "forward") == 0)   cmd.type = CommandType::Forward;
        else if (strcmp(direction, "backward") == 0)  cmd.type = CommandType::Backward;
        else if (strcmp(direction, "left") == 0)      cmd.type = CommandType::TurnLeft;
        else if (strcmp(direction, "right") == 0)     cmd.type = CommandType::TurnRight;
        else                                            cmd.type = CommandType::Unknown;
    } else if (strcmp(type, "stop") == 0) {
        cmd.type = CommandType::Stop;
    } else if (strcmp(type, "servo") == 0) {
        cmd.type = CommandType::Servo;
        cmd.servoChannel = doc["channel"] | 0;
        cmd.servoAngle   = doc["angle"] | 0;
    } else if (strcmp(type, "emergency_stop") == 0) {
        cmd.type = CommandType::EmergencyStop;
    } else if (strcmp(type, "clear_emergency_stop") == 0) {
        cmd.type = CommandType::ClearEmergencyStop;
    }

    return cmd;
}

void CommandProcessor::handleMessage(const char* topic, const uint8_t* payload, unsigned int length) {
    RobotCommand cmd = parse(payload, length);
    if (cmd.type == CommandType::Unknown) {
        Logger::log(LogLevel::Warn, "CommandProcessor", "Commande inconnue ou malformee, ignoree");
        return;
    }
    execute(cmd);
}

void CommandProcessor::execute(const RobotCommand& cmd) {
    // L'emergency stop a une priorité absolue : on attend le mutex sans timeout court,
    // car cette commande DOIT s'exécuter, contrairement aux commandes de mouvement normales.
    TickType_t waitTicks = (cmd.type == CommandType::EmergencyStop) ? portMAX_DELAY : pdMS_TO_TICKS(200);

    if (xSemaphoreTake(_robotMutex, waitTicks) != pdTRUE) {
        Logger::log(LogLevel::Warn, "CommandProcessor", "Mutex robot indisponible, commande ignoree");
        return;
    }

    switch (cmd.type) {
        case CommandType::Forward:   _robot.forward(cmd.speed); break;
        case CommandType::Backward:  _robot.backward(cmd.speed); break;
        case CommandType::TurnLeft:  _robot.turnLeft(cmd.speed); break;
        case CommandType::TurnRight: _robot.turnRight(cmd.speed); break;
        case CommandType::Stop:      _robot.stop(); break;
        case CommandType::Servo:     _robot.moveServo(cmd.servoChannel, cmd.servoAngle); break;
        case CommandType::EmergencyStop:      _robot.emergencyStop(); break;
        case CommandType::ClearEmergencyStop: _robot.clearEmergencyStop(); break;
        default: break;
    }

    xSemaphoreGive(_robotMutex);
    Logger::log(LogLevel::Info, "CommandProcessor", "Commande executee");
}