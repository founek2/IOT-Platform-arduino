#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <map>
#include <ArduinoJson.h>
#include <TickerScheduler.h>
#include <Ota.h>

#ifndef IOTPlatforma_H
#define IOTPlatforma_H

#define maxLen 2 + 1 + 32 + 100 + 1
#define CALLBACK_SIGNATURE std::function<void(char *, uint8_t *, unsigned int)>
#define MY_CALLBACK_SIGNATURE std::function<void(JsonVariant &)>
#define EEPROM_LEN 256

class IOTPlatforma
{
    const char *API_KEY;
    WiFiClientSecure wifiClient;
    PubSubClient client;
    const int topicMaxLen = maxLen;
    static TickerScheduler timer(1);
    bool shouldSendAck = false; // It is currently not recommended to do blocking IO operations from Ticker
    Ota ota;
    String deviceTopic;

    uint addr = 0;

    uint mode;

    std::map<String, MY_CALLBACK_SIGNATURE> handleControl;
    CALLBACK_SIGNATURE defaultCallback;

    /**
     * Download topic from https server with device by API_KEY
    */
    bool downloadTopic(String &, int len = maxLen);

    /**
     * Check if topic is for control -> localHandle, else -> defaultHandler
     */
    void handleSubscribe(char *topic, uint8_t *payload, unsigned int len);

    /**
    * Load topic from EEPROM or download it and save it
    */
    void getTopic();

      /**
     * Connect to mqtt broker
     */
    bool connect(int counter = 5);

    bool loadTopic();
     void writeTopic(const char deviceTopic[]);
public:
    IOTPlatforma(const char *apiKey, uint my_mode = IOTPlatforma::SENSOR_MODE);

    /**
     * Connect to wifi with provided credentials
     */
    bool wifi(const char *SSID, const char *password, int retry = 15);

    /**
     * Initialize topic and connect to mqtt broker
     */
    void init();

      /**
     * Loop for handling mqtt connection 
     */
    void loop();

    /**
     * Send data as Json {key: value} to broker
     */
    bool send(const float data[], const String keys[], const int count);

    /**
     * Subscribe to provided topic
     */
    boolean subscribe(const char *topic);
    boolean subscribe(const char *topic, int qos);

    /**
     * Subscribe to be controlled
     */
    boolean subscribeControl();

    /**
     * Unsubscribe from provided topic
     */
    boolean unSubscribe(const char *topic);

    /**
     * Set default callback for incoming messages
     */
    void setCallback(MQTT_CALLBACK_SIGNATURE);

    /**
     * Set callback for specific Json key
     */
    void setCallback(const char *key, MY_CALLBACK_SIGNATURE);

    /**
     * Delete saved topic from EEPROM
     */
    void clearTopic();

    /**
     * Save manualy topic into EEPROM
     */
    void saveTopic(const char * deviceTopic);

    /**
     * Send init control values to broker
     */
    bool sendInitControl(const int data[], const String keys[], const int count);

        /**
     * Send init control values to broker
     */
    bool sendInitControl(const String data[], const String keys[], const int count);

    /**
     * Send ack
     */
    bool sendAck(uint8_t *payload, unsigned int len);

    /**
     * Send empty ack
     */
    void sendEmptyAck();

    /**
     * Check connection to broker
     */
    boolean connected(void);

    static const uint CONTROL_MODE = 1;

    static const uint SENSOR_MODE = 0;

    void enableOTA(const char * password, const size_t port = 8266, const char *hostname = nullptr  );
};

#endif