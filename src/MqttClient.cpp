#include "MqttClient.h"
#include <Arduino.h>
#include "Logger.h"

static MqttClient::MessageCallback g_userCallback;

static void staticMqttCallback(char* topic, uint8_t* payload, unsigned int length) {
    if (g_userCallback) g_userCallback(topic, payload, length);
}

MqttClient::MqttClient(const char* host, uint16_t port, const char* clientId)
    : _mqtt(_wifiClient), _host(host), _port(port), _clientId(clientId) {}

void MqttClient::setStatusTopic(const char* topic) {
    _statusTopic = topic;
}

void MqttClient::setOnMessage(MessageCallback cb) {
    g_userCallback = cb;
}

void MqttClient::begin() {
    _mqtt.setServer(_host, _port);
    _mqtt.setCallback(staticMqttCallback);
}

void MqttClient::attemptConnect(uint32_t nowMs) {
    Logger::log(LogLevel::Info, "MQTT", "Tentative de connexion...");

    bool ok;
    if (_statusTopic) {
        // Last Will : si le robot se déconnecte brutalement, le broker publie "offline" à sa place.
        ok = _mqtt.connect(_clientId, _statusTopic, 1, true, "offline");
    } else {
        ok = _mqtt.connect(_clientId);
    }

    if (ok) {
        Logger::log(LogLevel::Info, "MQTT", "Connecte");
        if (_statusTopic) {
            publish(_statusTopic, "online", true);
        }
        _backoffMs = 1000;
    } else {
        Logger::log(LogLevel::Warn, "MQTT", "Echec connexion");
        _backoffMs = min(_backoffMs * 2, (uint32_t)30000);
    }
    _lastAttemptMs = nowMs;
}

void MqttClient::loop() {
    if (!_mqtt.connected()) {
        uint32_t now = millis();
        if (now - _lastAttemptMs >= _backoffMs) {
            attemptConnect(now);
        }
        return;
    }
    _mqtt.loop();
}

bool MqttClient::isConnected() {
    return _mqtt.connected();
}

bool MqttClient::publish(const char* topic, const char* payload, bool retained) {
    if (!_mqtt.connected()) return false;
    return _mqtt.publish(topic, payload, retained);
}

bool MqttClient::subscribe(const char* topic) {
    if (!_mqtt.connected()) return false;
    return _mqtt.subscribe(topic);
}