#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <IOTPlatforma.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

const char fingerprint[] = "7C 77 E9 A0 BB 42 C5 1D 09 1B E4 BF 7F 9E 32 A7 54 AD 4C 19";
const char *mqtt_server = "192.168.10.202";
// const char *mqtt_server = "test.iotplatforma.cloud";

IOTPlatforma::IOTPlatforma(const char *apiKey) : API_KEY(apiKey), wifiClient(), client(wifiClient), initialized(false)
{
    wifiClient.setFingerprint(fingerprint);
}

/**
 * Init
 * Load topic from EEPROM or download it and save
*/
void IOTPlatforma::init()
{
    EEPROM.begin(256);
    EEPROM.get(addr, data);
    // Serial.println("topic is " + String(data.str));

    if (strncmp(data.str, "00/", 3) == 0)
    {
        Serial.print("topic is: ");
        Serial.println(data.str + 2);
    }
    else
    {
        Serial.println("Old values are: " + String(data.val) + "," + String(data.str));
        Serial.println("Must download topic");
        // strncpy(data.str, "00/api/kekel/ble", 100);
        char topic[maxLen];
        if (downloadTopic(topic))
        {
            Serial.println(topic);

            data.val = 0;
            strncpy(data.str, "00", 3);
            strncpy(data.str + 2, topic, maxLen - 2);
            Serial.println(data.str);
            EEPROM.put(addr, data);
            EEPROM.commit();
            // EEPROM.end();
        }
        else
        {
            return;
            Serial.println("Downloading topic failed");
        }
    }
    initialized = true;
    client.setServer(mqtt_server, 8883);
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
bool IOTPlatforma::connect()
{
    // TODO check for topic -> otherwise download it!
    randomSeed(micros());
    int counter = 0;
    while (!client.connected() && counter < 5)
    {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);
        ++counter;
        if (client.connect(clientId.c_str(), API_KEY, "kekel"))
        {
            Serial.println("connected");
            return true;
            // Once connected, publish an announcement...
            // client.publish("/kekel/outTopic", "hello world");
            // ... and resubscribe
            // client.subscribe("/kekel/inTopic");
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
    client.loop();
}

bool IOTPlatforma::downloadTopic(char *out, int len)
{
    std::unique_ptr<BearSSL::WiFiClientSecure> clientInsecure(new BearSSL::WiFiClientSecure);

    clientInsecure->setInsecure();
    HTTPClient https;
    if (https.begin(*clientInsecure, "https://test.iotplatforma.cloud/api/iot/topic?API_KEY=" + String(API_KEY)))
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
                String payload = https.getString();
                Serial.println("Downloaded topic = " + payload);
                https.end();
                strncpy(out, payload.c_str(), len);
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

bool IOTPlatforma::send(float values[], String keys[], int count) {
    if (!initialized) return false;
    
    String json = "{";
    for (int i = 0; i < count; ++i) {
        json += "\"" + keys[i] + "\":" + round(values[i]);
        if (i != count - 1) json += ",";
    }
    json += "}";
    client.publish(topic, data.c_str());
}