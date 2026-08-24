#pragma once
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <functional>

class MqttClient {
public:
    using MessageCallback = std::function<void(const char* topic, const uint8_t* payload, unsigned int length)>;
    using ConnectedCallback = std::function<void()>;

    MqttClient(const char* host, uint16_t port, const char* clientId);

    void setStatusTopic(const char* topic);
    void setOnMessage(MessageCallback cb);
    void setOnConnected(ConnectedCallback cb); //  appelé après chaque (re)connexion réussie
    void begin();
    void loop();
    bool isConnected();
    bool publish(const char* topic, const char* payload, bool retained = false);
    bool subscribe(const char* topic);

private:
    void attemptConnect(uint32_t nowMs);

    WiFiClient   _wifiClient;
    PubSubClient _mqtt;
    const char*  _host;
    uint16_t     _port;
    const char*  _clientId;
    const char*  _statusTopic = nullptr;
    ConnectedCallback _onConnected;
    uint32_t     _lastAttemptMs = 0;
    uint32_t     _backoffMs = 1000;
};