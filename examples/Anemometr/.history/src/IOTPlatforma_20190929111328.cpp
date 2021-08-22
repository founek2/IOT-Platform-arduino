#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <IOTPlatforma.h>

const char fingerprint[] = "7C 77 E9 A0 BB 42 C5 1D 09 1B E4 BF 7F 9E 32 A7 54 AD 4C 19";
const char *mqtt_server = "192.168.10.202";
// const char *mqtt_server = "test.iotplatforma.cloud";

WiFiClientSecure wifiClient;
PubSubClient client(wifiClient);

class IOTPlatforma
{
    const char *API_KEY;

public:
    IOTPlatforma(const char *apiKey) : API_KEY(apiKey)
    {
        wifiClient.setFingerprint(fingerprint);
        client.setServer(mqtt_server, 8883);
    }
    bool wifi(const char *SSID, const char *password, int retry = 15)
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

        if (counter >= retry) return false;

        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        return true;
    }
    bool connect()
    {
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
};