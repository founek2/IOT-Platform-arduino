#include <MqttClient.h>

// const char fingerprint[] = "7C:77:E9:A0:BB:42:C5:1D:09:1B:E4:BF:7F:9E:32:A7:54:AD:4C:19"; // localhost
// const char fingerprint[] = "90:72:F4:F6:D6:4D:EA:76:6C:B7:14:B3:DD:CE:CC:DD:8E:F7:4E:E7"; // v2.iot
// const char fingerprint[] = "62:5B:96:DF:D5:BE:4B:7C:44:FE:DF:68:AF:8D:8D:8D:6C:A6:DA:E6"; // test.iot
const char fingerprint[] = "B0:F6:60:AF:B6:21:6A:B1:19:4D:41:D3:37:DE:50:5D:B2:1F:96:9A"; // home.iot
#ifdef ESP32
const char ca_cert[] =
    "-----BEGIN CERTIFICATE-----"
    "MIIDUDCCAjigAwIBAgIUWEUYJSA39fZ308COIeCjAmpbivwwDQYJKoZIhvcNAQEL"
    "BQAwMTEgMB4GA1UEAwwXVExTR2VuU2VsZlNpZ25lZHRSb290Q0ExDTALBgNVBAcM"
    "BCQkJCQwHhcNMjEwOTE3MTI0MTU2WhcNMzEwOTE1MTI0MTU2WjAxMSAwHgYDVQQD"
    "DBdUTFNHZW5TZWxmU2lnbmVkdFJvb3RDQTENMAsGA1UEBwwEJCQkJDCCASIwDQYJ"
    "KoZIhvcNAQEBBQADggEPADCCAQoCggEBAM/brwaYrf0U4azzRhJCrdCSYmL4WgAd"
    "TSiwA6/RYB5I0u4Yq7vhbgC8bVaqjhVrxTnT1ml7ZM0DH9mW+NPDs5LhLSu77rBM"
    "qivbSuymLWbY8VR+YEJzUuC7O/wApAlXXTuvhUTduJwhpC81cTObBa/1obKFN2/S"
    "M/zw6TytEULxU6aPZ1IWMclZfCJN42VyicGNH9C8rcd9G6bW5mTsO0foPCsCD6V3"
    "kIoGzPJOKF1gQZRLe/sKVE4fwQQdNmig8Ro9mgpCSnRwUJ6KianpZSTzwcXOj3CP"
    "d+IteGjtTVbCFvIht5mhnX77YPn6GZGxdvQbdMiAPHbamcL1FqbrTRMCAwEAAaNg"
    "MF4wCwYDVR0PBAQDAgEGMB0GA1UdDgQWBBRowwh60TNW5iVa0rC39o4rOIeD3TAf"
    "BgNVHSMEGDAWgBRowwh60TNW5iVa0rC39o4rOIeD3TAPBgNVHRMBAf8EBTADAQH/"
    "MA0GCSqGSIb3DQEBCwUAA4IBAQB7IYxkwaulDr25iiqpvIIjWi/AwbPXXux61Kx+"
    "PRC460QBfFnTyMxdYmnNBGqmcRqikIF7GCmtVqIT0YHTyJ/4jWz12Bs5l5IqH/k2"
    "aJzH4wTG5cWoTF1jqxwYt9rwnRmEYt/0z2ay127A/+gNCTUmvWJHpCgbjoRJeWB7"
    "fvEUfxak/glZHi0DTDL/13lYzxrwl1jPz4WUPENFNao/2sxhz9sUEFDazQmaY8G6"
    "FRfS9v0M5ApHQxt0MxtkRpcMVR3869j9Ej9WnNaWNfbPatUQ1g3oyfHFifwmrV0s"
    "AwUW4GOPtsTVuYb/cLJsMerUORt6xW4j8t3b9KTR7Qz36+KC"
    "-----END CERTIFICATE-----";
#endif

MqttClient::MqttClient(const int bufferSize) : wifiClient(),
#ifdef ESP32
                                               client(bufferSize)
#else
                                               client(this->wifiClient)
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
    if (strcmp(server, "iotdomu.cz") == 0)
#ifdef ESP32
        this->wifiClient.setCACert(ca_cert);
#else
        this->wifiClient.setFingerprint(fingerprint);
#endif

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