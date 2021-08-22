#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <IOTPlatforma.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>

const char fingerprint[] = "7C 77 E9 A0 BB 42 C5 1D 09 1B E4 BF 7F 9E 32 A7 54 AD 4C 19"; // localhost
// const char fingerprint[] = "CE:EF:7D:A2:87:30:E1:CD:AF:91:2C:B2:18:EF:72:51:02:35:AB:F9"; // test.iot
const char *mqtt_server = "192.168.10.202";
// const char *mqtt_server = "test.iotplatforma.cloud";
//String iot_api = "http://192.168.10.202:8085";
String iot_api = "https://test.iotplatforma.cloud";

IOTPlatforma::IOTPlatforma(const char *apiKey, uint my_mode) : API_KEY(apiKey), wifiClient(), client(wifiClient), mode(my_mode)
{
    wifiClient.setFingerprint(fingerprint);
}

void IOTPlatforma::clearTopic()
{
    EEPROM.begin(EEPROM_LEN);
    for (unsigned int i = 0; i < EEPROM_LEN; i++)
    {
        EEPROM.write(i, 0);
    }
    EEPROM.end();
}

void IOTPlatforma::init()
{
    getTopic();
    if (mode == IOTPlatforma::CONTROL_MODE)
    {
        client.setCallback([this](char *topic, byte *payload, unsigned int length) {
            this->handleSubscribe(topic, payload, length);
        });
        this->timer_ack.attach(5 * 60, [this] { this->sendEmptyAck(); });
    }

    client.setServer(mqtt_server, 8883);
    this->connect();
    this->sendEmptyAck();
}

void IOTPlatforma::getTopic()
{
    EEPROM.begin(EEPROM_LEN);
    char data[maxLen];
    EEPROM.get(addr, data);
    // Serial.println("topic is " + String(data.str));

    if (strncmp(data, "77/", 3) == 0)
    {
        Serial.print("topic is: ");
        Serial.println(data + 2);
        this->deviceTopic = data + 2;
    }
    else
    {
        Serial.println("Must download topic");

        if (downloadTopic(deviceTopic))
        {
            Serial.println(deviceTopic);

            strncpy(data, "77", 3);
            strncpy(data + 2, deviceTopic.c_str(), maxLen - 2);
            Serial.println(data);
            EEPROM.put(addr, data);
            EEPROM.commit();
            EEPROM.end();
        }
        else
        {
            return;
            Serial.println("Downloading topic failed");
        }
    }
}

bool IOTPlatforma::wifi(const char *SSID, const char *password, int retry)
{
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(SSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, password);

    int counter = 0;
    while (WiFi.status() != WL_CONNECTED && counter < retry)
    {
        delay(500);
        Serial.print(".");
        ++counter;
    }

    if (counter >= retry)
        return false;

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    return true;
}
bool IOTPlatforma::connect(int counter)
{
    // TODO check for topic -> otherwise download it!
    randomSeed(micros());
    while (!client.connected() && counter > 0)
    {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);
        --counter;
        if (client.connect(clientId.c_str(), API_KEY, "kekel"))
        {
            Serial.println("connected");

            if (this->mode == IOTPlatforma::CONTROL_MODE)
                this->subscribeControl();
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

void IOTPlatforma::loop()
{
    if (!client.connected())
        Serial.println("MQTT disconnected");
    //this->connect();
    client.loop();
}

bool IOTPlatforma::downloadTopic(String &out, int len)
{
    std::unique_ptr<BearSSL::WiFiClientSecure> clientInsecure(new BearSSL::WiFiClientSecure);
    clientInsecure->setInsecure();

    // WiFiClient client; // localOnly
    HTTPClient https; // TODO https
    Serial.print("down topic from: ");
    Serial.println(iot_api + "/api/iot/topic?API_KEY=" + API_KEY);
    if (https.begin(*clientInsecure, iot_api + "/api/iot/topic?API_KEY=" + API_KEY))
    { // HTTPS

        Serial.print("[HTTPS] GET...\n");
        // start connection and send HTTP header
        int httpCode = https.GET();

        // httpCode will be negative on error
        if (httpCode > 0)
        {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

            // file found at server
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
            {
                out = https.getString();
                Serial.println("Downloaded topic = " + out);
                https.end();

                return true;
            }
        }
        else
        {
            Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
        }

        https.end();
    }
    else
    {
        Serial.printf("[HTTPS] Unable to connect\n");
    }

    return false;
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
    return client.publish(this->deviceTopic.c_str(), json.c_str());
}

bool IOTPlatforma::sendInitControl(const int values[], const String keys[], const int count)
{
    if (!client.connected())
        return false;

    String json = "{";
    for (int i = 0; i < count; ++i)
    {
        json += "\"" + keys[i] + "\":" + values[i];
        if (i != count - 1)
            json += ",";
    }
    json += "}";
    Serial.print("Sending control json: ");
    Serial.println(json);
    return client.publish((this->deviceTopic + "/initControl").c_str(), json.c_str());
}

bool IOTPlatforma::subscribe(const char *topic)
{
    return client.subscribe(topic);
}

bool IOTPlatforma::subscribeControl()
{
    //return client.subscribe((this->deviceTopic + "/update").c_str(), 1);
    Serial.println("subscribing to " + (this->deviceTopic + "/update"));
    return client.subscribe((this->deviceTopic + "/update").c_str());
}

bool IOTPlatforma::subscribe(const char *topic, int qos)
{
    return client.subscribe(topic, qos);
}

bool IOTPlatforma::unSubscribe(const char *topic)
{
    return client.unsubscribe(topic);
}

void IOTPlatforma::setCallback(MQTT_CALLBACK_SIGNATURE)
{
    defaultCallback = callback;
}

void IOTPlatforma::setCallback(const char *key, MY_CALLBACK_SIGNATURE callback)
{
    handleControl.emplace(key, callback);
}

boolean IOTPlatforma::connected(void)
{
    return client.connected();
}

bool IOTPlatforma::sendAck(uint8_t *payload, unsigned int len)
{
    if (client.connected())
        return client.publish((String(this->deviceTopic) + "/ack").c_str(), payload, len);
    else
        return false;
}

void IOTPlatforma::sendEmptyAck()
{
    if (client.connected())
    {
        Serial.println("sending empty ack");
        String ack_json = "{\"ack\": 1}";
        client.publish((String(this->deviceTopic) + "/ack").c_str(), ack_json.c_str());
    }
}

void IOTPlatforma::handleSubscribe(char *topic, uint8_t *payload, unsigned int len)
{
    uint8_t originalPayload[len];
    memcpy(originalPayload, payload, len / sizeof(payload[0]));
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (unsigned int i = 0; i < len; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    // check for control topic -> otherwise defaultCallback
    const String updateTopic = this->deviceTopic + "/update";
    if (strncmp(updateTopic.c_str(), topic, updateTopic.length()) != 0)
        this->defaultCallback(topic, payload, len);

    const size_t capacity = JSON_OBJECT_SIZE(1) + 20;
    StaticJsonDocument<capacity> doc;

    DeserializationError error = deserializeJson(doc, payload, len);

    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    }

    if (doc.size() == 1)
    {
        for (auto it = handleControl.begin(); it != handleControl.end(); ++it)
        {
            if (doc.containsKey(it->first))
            {
                Serial.print("json obj: " + it->first + ":");
                Serial.println(doc[it->first].as<int>());
                JsonVariant js = doc[it->first]; // must make manual conversion, otherwise it does not work

                //client.publish((String(topic) + "/ack").c_str(), originalPayload, len);
                this->sendAck(originalPayload, len);

                this->handleControl.at(it->first)(js);
                return;
            }
        }
    }
    else
        Serial.println("json has size != 1");
}