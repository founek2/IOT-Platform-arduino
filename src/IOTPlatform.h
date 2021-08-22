#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <Ota.h>
#include <WiFiManager.h>
#include <Memory.h>
#include <Device.h>

#ifndef IOTPlatform_H
#define IOTPlatform_H

class IOTPlatform
{
    WiFiClientSecure wifiClient;
    PubSubClient client;
    Ota ota;
    Memory mem;
    Device _device;

    /**
     * Check if topic is for control -> localHandle, else -> defaultHandler
     */
    void _handleSubscribe(const char *topic, byte *payload, unsigned int len);

    /**
     * Connect to mqtt broker
     */
    bool _connectDiscovery();
    bool _connectPaired();
    bool _connectWith(const char *userName, const char *password, int counter = 2);
    bool _connect();
    bool _userExists(const char *userName, const char *server);

public:
    IOTPlatform(const char *deviceName);

    Node *NewNode(const char *nodeId, const char *name, NodeType = NodeType::GENERIC);

    /**
     * Connect to wifi with provided credentials
     */
    bool captivePortal(const char *SSID = "Nastav mě");

    /**
     * Initialize topic and connect to mqtt broker
     */
    void start();

    /**
     * Loop for handling mqtt connection 
     */
    void loop();

    /**
     * Delete saved topic from EEPROM
     */
    void reset();

    /**
     * Check connection to broker
     */
    boolean connected(void);

    void enableOTA(const char *password, const uint16_t port = 8266, const char *hostname = nullptr);

    void disconnect();

    void sleep();

    bool publish(const char *topic, const char *message);
};

#endif