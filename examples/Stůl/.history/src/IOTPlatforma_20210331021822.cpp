#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <IOTPlatforma.h>
#include <WiFiClientSecureBearSSL.h>
#include <WiFiManager.h>

// const char fingerprint[] = "7C:77:E9:A0:BB:42:C5:1D:09:1B:E4:BF:7F:9E:32:A7:54:AD:4C:19"; // localhost
// const char fingerprint[] = "90:72:F4:F6:D6:4D:EA:76:6C:B7:14:B3:DD:CE:CC:DD:8E:F7:4E:E7"; // v2.iot
const char fingerprint[] = "62:5B:96:DF:D5:BE:4B:7C:44:FE:DF:68:AF:8D:8D:8D:6C:A6:DA:E6"; // test.iot

WiFiManager wifiManager;

bool IOTPlatforma::captivePortal(const char *SSID)
{
    // this->mem.clearEEPROM();
    // wifiManager.erase();

    wifiManager.setShowInfoUpdate(false);
    WiFiManagerParameter custom_username("username", "Uživ. jméno", "martas", realmMaxLen - 1);
    wifiManager.addParameter(&custom_username);

    WiFiManagerParameter custom_mqtt_server("server", "Platforma URL", "dev.iotplatforma.cloud", serverMaxLen - 1);
    wifiManager.addParameter(&custom_mqtt_server);

    bool shouldSaveParams = false;
    wifiManager.setSaveParamsCallback([&shouldSaveParams]() {
        Serial.println("setSaveParamsCallback");
        shouldSaveParams = true;
    });

    bool statusOk = false;
    while (statusOk == false)
    {
        wifiManager.autoConnect(SSID);

        Serial.print("shouldSaveParams");
        Serial.println(shouldSaveParams);
        if (shouldSaveParams)
        {
            this->mem.setServerAndRealm(custom_mqtt_server.getValue(), custom_username.getValue());
            this->_device.setRealm(this->mem.getRealm());
        }
        else
        {
            this->mem.loadEEPROM();
            this->_device.setRealm(this->mem.getRealm());
        }

        if (this->mem.getPairStatus() != PAIR_STATUS_BLANK)
        {
            statusOk = true;
        }
        else
            wifiManager.erase();
    }

    // this->mem.setServerAndRealm("dev.iotplatforma.cloud", "martas");
    // this->_device.setRealm(this->mem.getRealm());
    return wifiManager.getWiFiIsSaved();
}

IOTPlatforma::IOTPlatforma(const char *deviceName) : wifiClient(), client(wifiClient), _device(deviceName, &this->client)
{

    wifiClient.setFingerprint(fingerprint);
    // wifiClient.setInsecure();
}

void IOTPlatforma::start()
{
    client.setServer(this->mem.getServer(), 8884);

    this->_connect();
}

void IOTPlatforma::_connect()
{
    if (this->mem.getPairStatus() == PAIR_STATUS_INIT)
    {
        this->_connectDiscovery();
    }
    else if (this->mem.getPairStatus() == PAIR_STATUS_PAIRED)
    {

        this->_connectPaired();
    }
    else
        Serial.println("Error> pairStatus should be at least PAIR_STATUS_INIT");
}

bool IOTPlatforma::_connectWith(const char *userName, const char *password, int counter)
{
    Serial.printf("_connectWith ID=%s, userName=%s, password=%s, server=%s\n", this->_device.getId(), userName, password, this->mem.getServer());

    randomSeed(micros());
    while (!client.connected() && counter > 0)
    {
        Serial.printf("FreeHeap %d\n", ESP.getFreeHeap());

        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);
        --counter;
        if (client.connect(clientId.c_str(), "device=martas/ESPDF1126", password, (this->_device.getTopic() + "/" + "$state").c_str(), 1, false, Status::lost))
        {
            Serial.println("connected");
            client.setCallback([this](const char *topic, byte *payload, unsigned int length) {
                this->_handleSubscribe(topic, payload, length);
            });
            return true;
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
    return false;
}

bool IOTPlatforma::_connectDiscovery()
{
    Serial.println("Connecting discovery");

    this->_device.setPrefix("prefix");
    bool connected = this->_connectWith((std::string("guest=") + this->_device.getId()).c_str(), "guest");
    if (connected)
    {
        this->client.subscribe((this->_device.getTopic() + "/$config/apiKey/set").c_str());
        this->_device.publishStatus(Status::init);
        this->_device.announce();
        this->_device.publishStatus(Status::ready);
    }
    return connected;
}

bool IOTPlatforma::_connectPaired()
{
    Serial.println("Connecting paired");

    this->_device.setPrefix((String("v2/") + this->mem.getRealm()).c_str());
    bool connected = this->_connectWith((String("device=") + this->mem.getRealm() + "/" + this->_device.getId()).c_str(), this->mem.getApiKey());
    if (connected)
    {
        this->_device.publishStatus(Status::ready);
        this->_device.subscribe();
    }

    return connected;
}

void IOTPlatforma::loop()
{
    if (!client.connected())
    {
        this->_connect();
    }
    client.loop();

    if (this->ota.enabled)
        this->ota.handleOTA();
}

bool IOTPlatforma::send(const float values[], const String keys[], const int count)
{
    if (!this->client.connected())
        return false;

    String json = "{";
    for (int i = 0; i < count; ++i)
    {
        json += "\"" + keys[i] + "\":" + values[i];
        if (i != count - 1)
            json += ",";
    }
    json += "}";
    Serial.print("Sending sensor json: ");
    Serial.println(json);
    return client.publish(this->_device.getTopic().c_str(), json.c_str());
}

bool IOTPlatforma::subscribe(const char *topic)
{
    return client.subscribe(topic);
}

bool IOTPlatforma::subscribe(const char *topic, int qos)
{
    return client.subscribe(topic, qos);
}

bool IOTPlatforma::unSubscribe(const char *topic)
{
    return client.unsubscribe(topic);
}

boolean IOTPlatforma::connected(void)
{
    return client.connected();
}

void IOTPlatforma::_handleSubscribe(const char *top, byte *payload, unsigned int len)
{
    String topic = top;
    payload[len] = '\0'; // possible error
    Serial.print("topic ");
    Serial.println(topic.c_str());
    Serial.print("payload ");
    Serial.println((char *)payload);

    String apiTopic = this->_device.getTopic() + "/$config/apiKey/set";

    Serial.printf("handle %s vs %s\n", this->_device.getTopic().c_str(), topic.c_str());
    if (apiTopic == topic)
    {
        char data[len + 1];
        memcpy(data, payload, len);
        data[len] = '\0';

        this->mem.setApiKey(data);
        this->_device.publishStatus(Status::paired);
        this->disconnect();
        this->_connectPaired();
    }
    else if (topic.startsWith(this->_device.getTopic()))
    {
        this->_device.handleSubscribe(topic, payload, len);
    }
}

void IOTPlatforma::enableOTA(const char *password, const size_t port, const char *hostname)
{
    this->ota.enableOTA(password, port, hostname);
}

Node *IOTPlatforma::NewNode(const char *nodeId, const char *name, const char *type)
{
    return this->_device.NewNode(nodeId, name, type);
}

void IOTPlatforma::disconnect()
{
    this->_device.publishStatus(Status::disconnected);
    this->client.disconnect();
}

void IOTPlatforma::reset()
{
    wifiManager.erase();
    this->mem.clearEEPROM();
}