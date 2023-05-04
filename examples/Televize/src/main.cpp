#include "Arduino.h"
#include "IOTPlatform.h"

// const char *ssid = "MujO2Internet_2.4G_C9A2B9";
// const char *password = "83902697";

IOTPlatform plat("Led TV");

bool RELAY_STATE = false;
int REALY_PIN = 0;
int LED_PIN = LED_BUILTIN;

void setup()
{
    Serial.begin(115200);
    // plat.reset();
    pinMode(REALY_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(REALY_PIN, LOW);

    // led
    digitalWrite(LED_PIN, LOW);

    Node *nodeSwitch = plat.NewNode("relay", "Led TV", NodeType::SWITCH);
    auto propSwitch = nodeSwitch->NewProperty("power", "Led TV", DataType::BOOLEAN);
    propSwitch->setSettable(true);
    propSwitch->setCallback([](Property *property)
                            {
                                Serial.printf("handling property value %s\n", property->getValue().c_str());

                                if (property->getValue() == "true")
                                    digitalWrite(REALY_PIN, HIGH);
                                else
                                    digitalWrite(REALY_PIN, LOW);
                            });

    // plat.clearTopic();
    // plat.saveTopic("/5d5554843beb3f419f9e4a64/garden/garage/gate");
    plat.enableOTA("123456777", 8266, "esp-lustr");
    plat.start();

    digitalWrite(LED_PIN, HIGH);
    propSwitch->setValue("false");
}

void loop()
{
    plat.loop();
}