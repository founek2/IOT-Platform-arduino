#include <MqttClient.h>

MqttClient::MqttClient(const int bufferSize) : wifiClient(),
#ifdef ESP32
                                               client(bufferSize)
#else
                                               client(this->wifiClient);
#endif
{
    this->bufferSize = bufferSize;
    this->wifiClient.setInsecure();

#ifdef ESP32
    this->client.begin(this->wifiClient);
#else
    this->client.setBufferSize(bufferSize);
#endif
}

bool MqttClient::connect(const char *clientID, const char *userName, const char *password, const char *willTopic, int willQoS, bool willRetain, const char *willMessage)
{
#ifdef ESP32
    this->client.setWill(willTopic, willMessage, willRetain, willQoS);
    return this->client.connect(clientID, userName, password);
#else
    return this->client.connect(clientID, userName, password, willTopic, willQoS, willRetain, willMessage);
#endif
}

bool MqttClient::publish(const char *topic, const char *payload)
{
#ifdef ESP32
    return this->client.publish(topic, payload);
#else
    return this->client.publish(topic, payload);
#endif
}

bool MqttClient::publish(const char *topic, const char *payload, boolean retained)
{
#ifdef ESP32
    return this->client.publish(topic, payload, retained, 1);
#else
    return this->client.publish(topic, payload, retained);
#endif
}

bool MqttClient::publish(const char *topic, const char *payload, boolean retained, int qos)
{
#ifdef ESP32
    return this->client.publish(topic, payload, retained, qos);
#else
    return this->client.publish(topic, payload, retained);
#endif
}

bool MqttClient::publish(const char *topic, const char *payload, unsigned int plength, boolean retained)
{
#ifdef ESP32
    return this->client.publish(topic, payload, plength, retained, 1);
#else
    return this->client.publish(topic, (const uint8_t *)payload, plength, retained);
#endif
}

bool MqttClient::publish(const char *topic, const char *payload, unsigned int plength, boolean retained, int qos)
{
#ifdef ESP32
    return this->client.publish(topic, payload, plength, retained, qos);
#else
    return this->client.publish(topic, (const uint8_t *)payload, plength, retained);
#endif
}
MqttClient *MqttClient::setServer(const char *server, int port)
{
#ifdef ESP32
    this->client.setHost(server, port);
#else
    this->client.setServer(server, port);
#endif

    return this;
}

bool MqttClient::subscribe(const char *topic)
{
    return this->client.subscribe(topic);
}

bool MqttClient::connected()
{
    return this->client.connected();
}

void MqttClient::disconnect()
{
    this->client.disconnect();
}
void MqttClient::setKeepAlive(int seconds)
{
    this->client.setKeepAlive(seconds);
}
void MqttClient::loop()
{
    this->client.loop();
}

void MqttClient::onMessage(MY_MQTT_CALLBACK_SIGNATURE)
{
    this->callback = callback;
#ifdef ESP32
    this->client.onMessageAdvanced([this](MQTTClient *client, char topic[], char bytes[], int length)
                                   { this->callback(topic, (byte *)bytes, length); });
#else
    this->client.setCallback(this->callback);
#endif
}

int MqttClient::state()
{
#ifdef ESP32
    return this->client.lastError();
#else
    return this->client.state();
#endif
}