#include "MqttClient.h"
#include <Arduino.h>
#include "Logger.h"

static MqttClient::MessageCallback g_userCallback;

static void staticMqttCallback(char* topic, uint8_t* payload, unsigned int length) {
    if (g_userCallback) g_userCallback(topic, payload, length);
}

MqttClient::MqttClient(const char* host, uint16_t port, const char* clientId, const char* rootCA)
    : _mqtt(_wifiClient), _host(host), _port(port), _clientId(clientId) {
    _wifiClient.setCACert(rootCA); // vérifie le certificat du broker -> pas de MITM silencieux
}

void MqttClient::setStatusTopic(const char* topic) { _statusTopic = topic; }
void MqttClient::setOnMessage(MessageCallback cb)   { g_userCallback = cb; }
void MqttClient::setOnConnected(ConnectedCallback cb) { _onConnected = cb; }

void MqttClient::begin() {
    _mqtt.setServer(_host, _port);
    _mqtt.setCallback(staticMqttCallback);
    _mqtt.setBufferSize(512); // les handshakes TLS + JSON signés dépassent le buffer par défaut (256)
}

void MqttClient::attemptConnect(uint32_t nowMs) {
    Logger::log(LogLevel::Info, "MQTT", "Tentative de connexion TLS...");

    bool ok = _statusTopic
        ? _mqtt.connect(_clientId, _statusTopic, 1, true, "offline")
        : _mqtt.connect(_clientId);

    if (ok) {
        Logger::log(LogLevel::Info, "MQTT", "Connecte (TLS)");
        if (_statusTopic) publish(_statusTopic, "online", true);
        if (_onConnected) _onConnected();
        _backoffMs = 1000;
    } else {
        // state() renvoie le code d'erreur PubSubClient : très utile pour diagnostiquer
        // un problème TLS (certificat expiré, horloge ESP32 pas synchronisée via NTP, etc.)
        char msg[64];
        snprintf(msg, sizeof(msg), "Echec connexion, code=%d", _mqtt.state());
        Logger::log(LogLevel::Warn, "MQTT", msg);
        _backoffMs = min(_backoffMs * 2, (uint32_t)30000);
    }
    _lastAttemptMs = nowMs;
}

void MqttClient::loop() {
    if (!_mqtt.connected()) {
        uint32_t now = millis();
        if (now - _lastAttemptMs >= _backoffMs) attemptConnect(now);
        return;
    }
    _mqtt.loop();
}

bool MqttClient::isConnected() { return _mqtt.connected(); }

bool MqttClient::publish(const char* topic, const char* payload, bool retained) {
    if (!_mqtt.connected()) return false;
    return _mqtt.publish(topic, payload, retained);
}

bool MqttClient::subscribe(const char* topic) {
    if (!_mqtt.connected()) return false;
    return _mqtt.subscribe(topic);
}