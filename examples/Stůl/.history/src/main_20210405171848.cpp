#include "IOTPlatforma.h"

bool RELAY_STATE = false;
int REALY_PIN = 12;
int LED_PIN = 13;

IOTPlatforma plat("Brána");

void setup()
{
    Serial.begin(115200);
    pinMode(REALY_PIN, OUTPUT);
    digitalWrite(REALY_PIN, LOW);

    // led
    pinMode(13, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    Node *gate = plat.NewNode("gate", "Brána", "activator");
    Property *prop = gate->NewProperty("power", "Zapnutí", "enum");
    prop->setFormat("on");
    prop->setSettable(true);
    prop->setCallback([](Property *prop) {
        digitalWrite(REALY_PIN, HIGH);
        digitalWrite(LED_PIN, LOW);

        delay(400);

        digitalWrite(LED_PIN, HIGH);
        digitalWrite(REALY_PIN, LOW);
    });

    plat.enableOTA("123456777");
    plat.start();

    digitalWrite(LED_PIN, HIGH);
}

void loop()
{
    plat.loop();
}
