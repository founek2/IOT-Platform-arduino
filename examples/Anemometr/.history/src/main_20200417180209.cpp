#include <ESP8266WiFi.h>

const char *SSID = "Zahrada";
const char *password = "1414elektron151";

const char *API_KEY = "fabd3f35584f8e4818a48eb3375119bc"; //v2.iotplatforma.cloud

bool RELAY_STATE = false;
int REALY_PIN = 12;
int LED_PIN = 13;

const String keys[] = {"switch"};
const String defaultData[] = {"{\"on\":1}"};

void setup() {
   Serial.begin(115200);
       delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(SSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, password);

    int counter = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
        ++counter;
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void loop() {
    Serial.println("sdads");
}
