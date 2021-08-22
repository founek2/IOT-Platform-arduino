#include "IOTPlatform.h"

bool RELAY_STATE = false;
bool reset_pressed = false;
int REALY_PIN = 12;
int LED_PIN = 13;
int BUTTON_PIN = 0;
IOTPlatform plat("Brána");

unsigned long buttonPressedAt = 0;
int button_state = 1;

void ICACHE_RAM_ATTR interrupt_routine()
{
    button_state = digitalRead(BUTTON_PIN);

    if (button_state == 0 && buttonPressedAt == 0)
    {
        buttonPressedAt = millis();
        Serial.println("Button pressed");
    }
    else if (button_state == 1)
    {
        Serial.printf("Diff %d, %d, %d", millis(), buttonPressedAt, millis() - buttonPressedAt);
        Serial.println("Button released");
        if (buttonPressedAt != 0 && millis() - buttonPressedAt - 5 * 1000 > 0)
            reset_pressed = true;

        buttonPressedAt = 0;
    }
}

void setup()
{
    Serial.begin(115200);
    // plat.reset();
    pinMode(REALY_PIN, OUTPUT);
    digitalWrite(REALY_PIN, LOW);

    // led
    pinMode(13, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // button
    digitalWrite(BUTTON_PIN, INPUT);
    attachInterrupt(BUTTON_PIN, interrupt_routine, CHANGE);

    Node *gate = plat.NewNode("gate", "Brána", NodeType::ACTIVATOR);
    Property *prop = gate->NewProperty("power", "Zapnutí", DataType::ENUM);
    prop->setFormat("on");
    prop->setSettable(true);
    prop->setCallback([](Property *prop)
                      {
                          digitalWrite(REALY_PIN, HIGH);
                          digitalWrite(LED_PIN, LOW);

                          delay(400);

                          digitalWrite(LED_PIN, HIGH);
                          digitalWrite(REALY_PIN, LOW);
                      });

    plat.enableOTA("123456777", 8266, "esp-brana");
    plat.start();

    digitalWrite(LED_PIN, HIGH);
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
