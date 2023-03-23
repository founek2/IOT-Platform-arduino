#ifdef ESP32
#include <MQTT.h>
#else
#include <PubSubClient.h>
#endif

#include <WiFiClientSecure.h>
#include <Base.h>
#include <Client.h>
#include <functional>
#define MY_MQTT_CALLBACK_SIGNATURE std::function<void(const char *, uint8_t *, unsigned int)> callback

#ifndef MqttClient_H
#define MqttClient_H

class MqttClient
{
#ifdef ESP32
    MQTTClient client;
#else
    PubSubClient client;
#endif

    MY_MQTT_CALLBACK_SIGNATURE;
    WiFiClientSecure wifiClient;
    int bufferSize;

public:
    MqttClient(const int bufferSize);

    /**
     * @brief Set connection target and validate certificate for known servers
     *
     * @param server domain name
     * @param port
     * @return MqttClient*
     */
    MqttClient *setServer(const char *server, int port);

    bool connect(const char *clientID, const char *userName, const char *password, const char *willTopic, int willQoS, bool willRetain, const char *willMessage);

    bool publish(const char *topic, const char *payload);
    bool publish(const char *topic, const char *payload, boolean retained);
    bool publish(const char *topic, const char *payload, boolean retained, int qos);
    bool publish(const char *topic, const char *payload, unsigned int plength, boolean retained);
    bool publish(const char *topic, const char *payload, unsigned int plength, boolean retained, int qos);
    bool subscribe(const char topic[]);
    bool connected();
    void disconnect();

    void onMessage(MY_MQTT_CALLBACK_SIGNATURE);

    void setKeepAlive(int seconds);
    void loop();
    int state();
};

#endif