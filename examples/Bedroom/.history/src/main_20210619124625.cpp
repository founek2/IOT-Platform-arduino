#include "IOTPlatforma.h"
#include "Arduino.h"

IOTPlatforma plat("Postel");
Property *propSwitch;
bool change = false;

const int LED_PIN = 15;
const int PIR_PIN = 13;

void ICACHE_RAM_ATTR interrupt_routine()
{
    change = true;
    Serial.println("interrupt");

    int pir_state = digitalRead(PIR_PIN);

    if (pir_state == 1)
        digitalWrite(LED_PIN, HIGH);
    else
        digitalWrite(LED_PIN, LOW);
}

void setup()
{
    pinMode(LED_PIN, OUTPUT);
    pinMode(PIR_PIN, INPUT);
    if (digitalRead(PIR_PIN))
        digitalWrite(LED_PIN, HIGH);
    else
        digitalWrite(LED_PIN, LOW);

    attachInterrupt(PIR_PIN, interrupt_routine, CHANGE);
    Serial.begin(115200);

    Node *nodeSwitch = plat.NewNode("relay", "Postel", NodeType::SWITCH);
    propSwitch = nodeSwitch->NewProperty("power", "Postel led", DataType::BOOLEAN);
    propSwitch->setSettable(true);
    propSwitch->setCallback([](Property *property) {
        if (property->getValue() == "true")
            digitalWrite(LED_PIN, HIGH);
        else
            digitalWrite(LED_PIN, LOW);
    });

    plat.enableOTA("123456777");
    plat.start();

    propSwitch->setValue("false");
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