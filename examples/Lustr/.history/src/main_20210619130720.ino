#include "IOTPlatforma.h"

// const char *ssid = "MujO2Internet_2.4G_C9A2B9";
// const char *password = "83902697";

IOTPlatforma plat("Lustr");

bool RELAY_STATE = false;
bool reset_pressed = false;
int REALY_PIN = 12;
int LED_PIN = 13;
int BUTTON_PIN = 0;

unsigned long buttonPressedAt = 0;

void ICACHE_RAM_ATTR interrupt_routine()
{
    int button_state = digitalRead(BUTTON_PIN);

    Serial.printf("interrupt, will reset? %d %d\n", millis() - buttonPressedAt - 5 * 1000, button_state);

    if (button_state == 0)
        buttonPressedAt = millis();
    else if (buttonPressedAt != 0 && millis() - buttonPressedAt - 5 * 1000 > 0)
    {
        Serial.println("int will reset!!!");
        buttonPressedAt = 0;
        reset_pressed = true;
    }
}

void setup()
{
    Serial.begin(115200);
    pinMode(REALY_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(BUTTON_PIN, INPUT);
    digitalWrite(REALY_PIN, HIGH);

    attachInterrupt(BUTTON_PIN, interrupt_routine, CHANGE);

    // led
    digitalWrite(LED_PIN, LOW);

    Node *nodeSwitch = plat.NewNode("relay", "Lustr", NodeType::SWITCH);
    auto propSwitch = nodeSwitch->NewProperty("power", "Lustr", DataType::BOOLEAN);
    propSwitch->setSettable(true);
    propSwitch->setCallback([](Property *property) {
        Serial.printf("handling property value %s\n", property->getValue().c_str());

        if (property->getValue() == "true")
            digitalWrite(REALY_PIN, HIGH);
        else
            digitalWrite(REALY_PIN, LOW);
    });

    auto toogleSwitch = nodeSwitch->NewProperty("toggle", "toogle", DataType::ENUM);
    toogleSwitch->setSettable(true);
    toogleSwitch->setFormat("on");
    toogleSwitch->setCallback([propSwitch](Property *property) {
        Serial.printf("handling property value %s\n", property->getValue().c_str());

        if (propSwitch->getValue() == "true")
        {
            digitalWrite(REALY_PIN, LOW);
            propSwitch->setValue("false");
        }
        else
        {
            digitalWrite(REALY_PIN, HIGH);
            propSwitch->setValue("true");
        }
    });
    // plat.clearTopic();
    // plat.saveTopic("/5d5554843beb3f419f9e4a64/garden/garage/gate");
    plat.enableOTA("123456777");
    plat.start();

    digitalWrite(LED_PIN, HIGH);
    propSwitch->setValue("true");
}

void loop()
{
    if (reset_pressed)
    {
        reset_pressed = false;
        Serial.println("reseting");
        plat.disconnect();
        plat.reset();
        digitalWrite(LED_PIN, LOW);
        delay(500);
        digitalWrite(LED_PIN, HIGH);
        ESP.restart();
    }

    plat.loop();
}