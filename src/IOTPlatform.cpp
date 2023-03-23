#include <EEPROM.h>
#include <IOTPlatform.h>
#include <WiFiManager.h>

WiFiManager wifiManager;

WiFiManagerParameter custom_username("username", "Uživ. jméno", "", realmMaxLen - 1);
WiFiManagerParameter custom_mqtt_server("server", "Platforma URL", "iotdomu.cz", serverMaxLen - 1);

const char *menu[] = {"wifi"};
void IOTPlatform::autoConnectAsync()
{
    Serial.println("Running autoConnectAsync");
    wifiManager.setConnectTimeout(10);

    this->wifiStatus = WifiStatus::CONNECTING;
    bool connected = wifiManager.autoConnect(this->SSID);

    if (connected)
    {
        this->wifiStatus = WifiStatus::CONNECTED;
    }
}
void IOTPlatform::_saveWifiParams()
{
    if (this->_userExists(custom_username.getValue(), custom_mqtt_server.getValue()))
    {
        Serial.println("Saving params");
        this->mem.setServerAndRealm(custom_mqtt_server.getValue(), custom_username.getValue());
        this->_device.setRealm(this->mem.getRealm());
        this->wifiStatus = WifiStatus::CONNECTED;
    }
    else
    {
        Serial.println("Invalid parameters provided");
        this->wifiStatus = WifiStatus::PARAMETERS_INVALID;
    }
}

IOTPlatform::IOTPlatform(const char *deviceName, bool disablePortalWhenPaired) : _disablePortalWhenPaired(disablePortalWhenPaired), wifiStatus(WifiStatus::DISCONNECTED), client(350), mem(), _device(deviceName, &this->client)
{
}

void IOTPlatform::_init()
{
    Serial.println("Running init method");
    WiFi.mode(WIFI_STA);
    randomSeed(micros());

    // Memory sync
    this->mem.init();
    this->mem.loadEEPROM();
    this->_device.setRealm(this->mem.getRealm());

    // Client Init
    this->client.setServer(this->mem.getServer(), 8883);
    this->client.setKeepAlive(15);
    this->client.onMessage([this](const char *topic, byte *payload, unsigned int length)
                           { this->_handleSubscribe(topic, payload, length); });

    // ------------------------------------------------------

    wifiManager.setMenu(menu, 1);
    // wifiManager.setConfigPortalTimeout(300);   // not working in non-blocking
    wifiManager.setShowInfoUpdate(false);
    wifiManager.addParameter(&custom_username);
    wifiManager.addParameter(&custom_mqtt_server);
    wifiManager.setConfigPortalBlocking(false);

    wifiManager.setSaveConfigCallback([this]()
                                      { this->wifiStatus = WifiStatus::PARAMETERS_TO_BE_SAVED; });
    wifiManager.setAPCallback([this](void *_)
                              {
                                Serial.print("setAPcallback");
                                
                                 this->wifiStatus = WifiStatus::CAPTIVE_PORTAL; });
    if (this->mem.getPairStatus() == PairStatus::PAIR_STATUS_PAIRED && this->_disablePortalWhenPaired)
        wifiManager.setEnableConfigPortal(false);

    this->_initialized = true;
}

bool IOTPlatform::_connect()
{
    if (this->mem.getPairStatus() == PAIR_STATUS_INIT)
    {
        return this->_connectDiscovery();
    }
    else if (this->mem.getPairStatus() == PAIR_STATUS_PAIRED)
    {
        return this->_connectPaired();
    }
    else
    {
        Serial.println("Error> pairStatus should be at least PAIR_STATUS_INIT, reseting wifi");
        wifiManager.erase();
        this->wifiStatus = WifiStatus::DISCONNECTED;
    }

    return false;
}

bool IOTPlatform::_connectWith(const char *userName, const char *password, const char *server, int counter)
{
    Serial.printf("_connectWith ID=%s, userName=%s, password=%s, server=%s\n", this->_device.getId(), userName, password, server);
    client.setServer(server, 8883);

    while (!client.connected() && counter > 0)
    {
        Serial.print("Attempting MQTT connection...");
        String clientId = String(this->_device.getId()) + "-" + this->mem.getRealm();

        --counter;
        if (client.connect(clientId.c_str(), userName, password, (this->_device.getTopic() + "/" + "$state").c_str(), 1, false, Status::lost))
        {
            Serial.println("connected");
            return true;
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.println(client.state());
        }
    }
    return false;
}

bool IOTPlatform::_connectDiscovery()
{
    Serial.println("Connecting discovery v2");

    this->_device.setPrefix("prefix");
    bool connected = this->_connectWith((String("guest=") + this->_device.getId()).c_str(), this->_device.getRealm(), this->mem.getServer());
    if (connected)
    {
        this->client.subscribe((this->_device.getTopic() + "/$config/apiKey/set").c_str());
        this->client.subscribe((this->_device.getTopic() + "/$cmd/set").c_str());
        this->_device.publishStatus(Status::init);
        this->_device.announce();
        this->_device.publishStatus(Status::ready);
    }
    return connected;
}

bool IOTPlatform::_connectPaired()
{
    Serial.println("Connecting paired");

    this->_device.setPrefix((String("v2/") + this->mem.getRealm()).c_str());
    bool connected = this->_connectWith((String("device=") + this->mem.getRealm() + "/" + this->_device.getId()).c_str(), this->mem.getApiKey(), this->mem.getServer());
    if (connected)
    {
        this->client.subscribe((this->_device.getTopic() + "/$cmd/set").c_str());
        this->_device.publishStatus(Status::init);
        this->_device.announceValues();
        this->_device.subscribe();
        this->_device.publishStatus(Status::ready);
    }

    return connected;
}

/* Calculate if should run another attempt
 * check milis() overflow, check: lastAttempt - now > interval */
bool shouldAttempt(unsigned long lastAttempt, unsigned long interval)
{
    auto now = millis();
    return lastAttempt > now || now - lastAttempt > interval;
}

unsigned long lastMqttConnectionAttempt = 0;
unsigned long lastWifiConnectAttempt = 0;
const unsigned long WIFI_RECONNECT_INTERVAL = 5 * 60 * 1000;
const unsigned long MQTT_RECONNECT_INTERVAL = 40 * 1000;

void IOTPlatform::loop()
{
    // first run just init everything, second run will proceed to connection
    if (!this->_initialized)
        return this->_init();

    wifiManager.process();

    switch (this->wifiStatus)
    {
    case WifiStatus::DISCONNECTED:
        Serial.println("Handeling disconnect");
        lastWifiConnectAttempt = millis();
        this->autoConnectAsync();
        break;
    case WifiStatus::PARAMETERS_TO_BE_SAVED:
        Serial.println("Handling save parameters");
        this->_saveWifiParams();
        break;
    case WifiStatus::PARAMETERS_INVALID:
        Serial.println("Handling invalid parameters");
        wifiManager.erase();
        this->autoConnectAsync();
        break;
    }

    if (this->wifiStatus == WifiStatus::CONNECTED && WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Handling WLAN disconnected");
        this->wifiStatus = WifiStatus::DISCONNECTED;
    }

    if ((this->wifiStatus == WifiStatus::CONNECTING || this->wifiStatus == WifiStatus::CAPTIVE_PORTAL) && shouldAttempt(lastWifiConnectAttempt, WIFI_RECONNECT_INTERVAL))
    {
        lastWifiConnectAttempt = millis();
        Serial.println("Handling reconnect");
        this->autoConnectAsync();
    }

    if (this->wifiStatus == WifiStatus::CONNECTED)
    {
        client.loop();

        if (!client.connected() && (lastMqttConnectionAttempt == 0 || shouldAttempt(lastMqttConnectionAttempt, MQTT_RECONNECT_INTERVAL)))
        {
            lastMqttConnectionAttempt = millis();
            Serial.print("Client disconnected, reconnecting. rc=");
            Serial.println(this->client.state());
            this->_connect();
        }

        if (this->ota.enabled)
            this->ota.handleOTA();
    }
}

boolean IOTPlatform::connected(void)
{
    return client.connected();
}

void IOTPlatform::_handleSubscribe(const char *top, byte *payload, unsigned int len)
{
    String topic = top;
    payload[len] = '\0'; // possible error, but since internal buffers are large, should be OK
    Serial.print("topic ");
    Serial.println(topic.c_str());
    Serial.print("payload ");
    Serial.println((char *)payload);

    String apiTopic = this->_device.getTopic() + "/$config/apiKey/set";
    String cmdTopic = this->_device.getTopic() + "/$cmd/set";

    Serial.printf("handle %s vs %s\n", this->_device.getTopic().c_str(), topic.c_str());
    if (apiTopic == topic)
    {
        char data[len + 1];
        memcpy(data, payload, len);
        data[len] = '\0';

        Serial.print("API_KEY");
        Serial.println(data);

        this->mem.setApiKey(data);
        this->_device.publishStatus(Status::paired);
        this->disconnect();
        this->_connectPaired();
    }
    else if (cmdTopic == topic)
    {
        if (String((const char *)payload) == "restart")
        {
            this->_device.publishStatus(Status::restarting);
            client.disconnect();
            ESP.restart();
        }
        else if (String((const char *)payload) == "reset")
        {
            this->disconnect();
            this->reset();
            ESP.restart();
        }
    }
    else if (topic.startsWith(this->_device.getTopic()))
    {
        this->_device.handleSubscribe(topic, payload, len);
    }
}

void IOTPlatform::enableOTA(const char *password, const uint16_t port, const char *hostname)
{
    this->ota.enableOTA(password, port, hostname);
}

Node *IOTPlatform::NewNode(const char *nodeId, const char *name, NodeType type)
{
    return this->_device.NewNode(nodeId, name, type);
}

void IOTPlatform::disconnect()
{
    this->_device.publishStatus(Status::disconnected);
    this->client.loop();
    this->client.disconnect();
}

void IOTPlatform::sleep()
{
    this->_device.publishStatus(Status::sleeping);
    this->client.disconnect();
}

void IOTPlatform::reset()
{
    wifiManager.erase();
    this->mem.clearEEPROM();
}

bool IOTPlatform::publish(const char *topic, const char *message)
{
    return this->client.publish(topic, message);
}

bool IOTPlatform::_userExists(const char *userName, const char *server)
{
    client.setServer(server, 8883);

    Serial.println("Checking user existence");
    if (this->_connectWith((String("guest=") + this->_device.getId()).c_str(), userName, server))
    {
        this->client.disconnect();
        return true;
    }

    return false;
}

bool IOTPlatform::isInit()
{
    return this->mem.isInit();
}

bool IOTPlatform::isPaired()
{
    return this->mem.isPaired();
}

ConnectionStatus IOTPlatform::state()
{
    if (this->wifiStatus == WifiStatus::CONNECTED)
        return ConnectionStatus::CONNECTED;
    else
        return ConnectionStatus::DISCONNECTED;
}