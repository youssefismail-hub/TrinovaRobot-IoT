#include "CommandProcessor.h"
#include <ArduinoJson.h>
#include <mbedtls/md.h>
#include "Logger.h"
#include "DiagnosticsService.h"

CommandProcessor::CommandProcessor(TrinovaRobot& robot, SemaphoreHandle_t robotMutex, const char* hmacSecret)
    : _robot(robot), _robotMutex(robotMutex), _hmacSecret(hmacSecret) {}

// Vérifie que le payload JSON contient un champ "sig" = HMAC-SHA256(reste_du_json, secret),
// hexadécimal en minuscules. Sans "sig" valide, la commande est rejetée -- point d'entrée
// réseau le plus exposé du système, donc aucune tolérance ici.
bool CommandProcessor::verifySignature(const uint8_t* payload, unsigned int length) const {
    StaticJsonDocument<256> doc;
    if (deserializeJson(doc, payload, length) != DeserializationError::Ok) return false;

    const char* providedSig = doc["sig"] | "";
    if (strlen(providedSig) != 64) { // SHA256 hex = 64 caracteres
        Logger::log(LogLevel::Warn, "CommandProcessor", "Signature absente ou taille invalide");
        return false;
    }

    // Recalcule le HMAC sur le JSON SANS le champ "sig" (retiré avant hachage,
    // convention : le client doit signer le même sous-document).
    doc.remove("sig");
    char canonical[256];
    size_t canonicalLen = serializeJson(doc, canonical, sizeof(canonical));

    uint8_t computedHmac[32];
    mbedtls_md_context_t ctx;
    const mbedtls_md_info_t* mdInfo = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mdInfo, 1);
    mbedtls_md_hmac_starts(&ctx, (const uint8_t*)_hmacSecret, strlen(_hmacSecret));
    mbedtls_md_hmac_update(&ctx, (const uint8_t*)canonical, canonicalLen);
    mbedtls_md_hmac_finish(&ctx, computedHmac);
    mbedtls_md_free(&ctx);

    char computedHex[65];
    for (int i = 0; i < 32; i++) sprintf(computedHex + i * 2, "%02x", computedHmac[i]);
    computedHex[64] = '\0';

    bool valid = (strcmp(computedHex, providedSig) == 0);
    if (!valid) {
        Logger::log(LogLevel::Error, "CommandProcessor", "Signature invalide - commande rejetee");
        DiagnosticsService::incrementErrorCount();
    }
    return valid;
}

RobotCommand CommandProcessor::parse(const uint8_t* payload, unsigned int length) {
    RobotCommand cmd;
    StaticJsonDocument<256> doc;
    if (deserializeJson(doc, payload, length) != DeserializationError::Ok) return cmd;

    const char* type = doc["type"] | "";

    if (strcmp(type, "move") == 0) {
        const char* direction = doc["direction"] | "";
        cmd.speed = doc["speed"] | 0;
        if      (strcmp(direction, "forward") == 0)  cmd.type = CommandType::Forward;
        else if (strcmp(direction, "backward") == 0) cmd.type = CommandType::Backward;
        else if (strcmp(direction, "left") == 0)     cmd.type = CommandType::TurnLeft;
        else if (strcmp(direction, "right") == 0)    cmd.type = CommandType::TurnRight;
        else                                           cmd.type = CommandType::Unknown;
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
    if (!verifySignature(payload, length)) {
        return; // rejet silencieux côté action, mais journalisé
    }

    RobotCommand cmd = parse(payload, length);
    if (cmd.type == CommandType::Unknown) {
        Logger::log(LogLevel::Warn, "CommandProcessor", "Commande inconnue ou malformee, ignoree");
        return;
    }
    execute(cmd);
}

void CommandProcessor::execute(const RobotCommand& cmd) {
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
    Logger::log(LogLevel::Info, "CommandProcessor", "Commande executee (signature validee)");
}