#include <ArduinoJson.h>
#include "IOTPlatforma.h"

const char *ssid = "Zahrada";
const char *password = "1414elektron151";

const char *API_KEY = "fabd3f35584f8e4818a48eb3375119bc"; //v2.iotplatforma.cloud

bool RELAY_STATE = false;
int REALY_PIN = 12;
int LED_PIN = 13;

void callbackSwitch(const JsonVariant &payload);
IOTPlatforma plat(API_KEY, IOTPlatforma::CONTROL_MODE);

const String keys[] = {"switch"};
const String defaultData[] = {"{\"on\":1}"};

void setup() {
     Serial.begin(115200);
   pinMode(REALY_PIN, OUTPUT);
   digitalWrite(REALY_PIN, HIGH);

   // led
   pinMode(13, OUTPUT);
   digitalWrite(LED_PIN, LOW);

   plat.wifi(ssid, password);
    plat.setCallback("switch", callbackSwitch);
    // plat.clearTopic();
    // plat.saveTopic("/5d5554843beb3f419f9e4a64/garden/garage/gate");
    plat.enableOTA("123456777");
    plat.init();

    plat.sendInitControl(defaultData, keys, 1);
    digitalWrite(LED_PIN, HIGH);
}

void loop() {
     plat.loop();
}

void callbackSwitch(const JsonVariant &jsonObj) // 1/0 || { type: "linear", color: #4242, on: 1, bright: 0-100}
{
    if (!jsonObj["on"].isNull())
    {
        int enabled = jsonObj["on"];

      Serial.println("got somesdsad");
        Serial.println(enabled);
        if (enabled)
           digitalWrite(REALY_PIN, HIGH);
        else
          digitalWrite(REALY_PIN, LOW);
    }
}