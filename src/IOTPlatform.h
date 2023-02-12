#include <Ota.h>
#include <Memory.h>
#include <Device.h>
#include <MqttClient.h>

#ifndef IOTPlatform_H
#define IOTPlatform_H

enum class WifiStatus
{
    DISCONNECTED,
    CAPTIVE_PORTAL,
    CAPTIVE_PORTAL_SHUTING_DOWN,
    PARAMETERS_TO_BE_SAVED,
    PARAMETERS_SAVED,
    PARAMETERS_INVALID,
    CONNECTING,
    CONNECTED,
};

enum class ConnectionStatus
{
    DISCONNECTED,
    CONNECTED,
};

class IOTPlatform
{
    WifiStatus wifiStatus;
    MqttClient client;
    Ota ota;
    Memory mem;
    Device _device;
    const char *SSID = "Nastav mě";
    /**
     * Check if topic is for control -> localHandle, else -> defaultHandler
     */
    void _handleSubscribe(const char *topic, byte *payload, unsigned int len);

    /**
     * Connect to mqtt broker
     */
    bool _connectDiscovery();
    bool _connectPaired();
    bool _connectWith(const char *userName, const char *password, const char *server, int counter = 2);
    bool _connect();
    bool _userExists(const char *userName, const char *server);
    void _saveWifiParams();

public:
    IOTPlatform(const char *deviceName);

    Node *NewNode(const char *nodeId, const char *name, NodeType = NodeType::GENERIC);

    /**
     * Connect to wifi with provided credentials
     */
    void autoConnectAsync();

    /**
     * Initialize topic and connect to mqtt broker
     */
    void init();

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

    /**
     * Whether API key was exchanged
     */
    bool isPaired();

    /**
     * Whether waiting to be add
     */
    bool isInit();

    ConnectionStatus state();
};

#endif