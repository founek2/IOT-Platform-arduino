#include "IOTPlatform.h"
#include "Arduino.h"

IOTPlatform plat("Postel");
Property *propSwitch;
bool change = false;

// const int LED_PIN = 15; // doma
const int LED_PIN = 0; // mauky
const int PIR_PIN = 13;

void ICACHE_RAM_ATTR interrupt_routine()
{
    change = true;
    Serial.println("interrupt");

    int pir_state = digitalRead(PIR_PIN);
    digitalWrite(LED_PIN, pir_state);
}

void setup()
{
    // plat.reset();
    pinMode(LED_PIN, OUTPUT);
    pinMode(PIR_PIN, INPUT);

    int pir_state = digitalRead(PIR_PIN);
    digitalWrite(LED_PIN, pir_state);

    attachInterrupt(PIR_PIN, interrupt_routine, CHANGE);
    Serial.begin(115200);

    Node *nodeSwitch = plat.NewNode("relay", "Postel", NodeType::SWITCH);
    propSwitch = nodeSwitch->NewProperty("power", "Postel led", DataType::BOOLEAN);
    propSwitch->setSettable(true);
    propSwitch->setCallback([](Property *property)
                            { digitalWrite(LED_PIN, property->getValue() == "true" ? HIGH : LOW); });

    plat.enableOTA("123456777");
    plat.start();

    propSwitch->setValue(pir_state ? "true" : "false");
}

int last_state = 0;

void loop()
{
    if (change)
    {
        change = false;
        int pir_state = digitalRead(PIR_PIN);
        if (pir_state != last_state)
        {
            last_state = pir_state;

            Serial.print("state: ");
            Serial.println(pir_state);
            if (pir_state == 1)
            {
                propSwitch->setValue("true");
            }
            else
            {
                propSwitch->setValue("false");
            }
        }
    }

    plat.loop();
}