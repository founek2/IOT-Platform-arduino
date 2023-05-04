#include "IOTPlatform.h"

bool RELAY_STATE = false;
int REALY_PIN = 12;
int LED_PIN = 13;
int BUTTON_PIN = 0;
IOTPlatform plat("Brána", true);

unsigned long buttonPressedAt = 0;
int volatile button_released = 1;
bool volatile resetDevice = false;
bool volatile quickButtonPress = false;

bool shouldAttemptDebounce(unsigned long lastAttempt, unsigned long interval)
{
    auto now = millis();
    return lastAttempt > now || now - lastAttempt > interval;
}

void IRAM_ATTR interrupt_routine()
{
    button_released = digitalRead(BUTTON_PIN);

    if (button_released == LOW && buttonPressedAt == 0)
        buttonPressedAt = millis();
    else if (button_released == HIGH && buttonPressedAt != 0)
    {
        if (shouldAttemptDebounce(buttonPressedAt, 5 * 1000))
            resetDevice = true;
        else
            quickButtonPress = true;

        buttonPressedAt = 0;
    }
}

void switchRelay()
{
    digitalWrite(REALY_PIN, HIGH);
    digitalWrite(LED_PIN, LOW);

    delay(400);

    digitalWrite(LED_PIN, HIGH);
    digitalWrite(REALY_PIN, LOW);
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
                        switchRelay();
                        return true; });

    plat.enableOTA("123456777", 8266, "esp-brana");

    digitalWrite(LED_PIN, HIGH);
}

void loop()
{
    if (quickButtonPress)
    {
        quickButtonPress = false;
        Serial.println("quick press");
        switchRelay();
    }
    else if (resetDevice)
    {
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
