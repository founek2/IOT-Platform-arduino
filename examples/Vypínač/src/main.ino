#include "IOTPlatform.h"

IOTPlatform plat("Vypínač");

bool RELAY_STATE = false;
bool reset_pressed = false;
int REALY_PIN = 12;
int LED_PIN = 13;
int BUTTON_PIN = 0;

int button_pressed = false;
#define DEBOUNCE_TIME 50 // the debounce time in millisecond, increase this time if it still chatters

int lastSteadyState = LOW;      // the previous steady state from the input pin
int lastFlickerableState = LOW; // the previous flickerable state from the input pin
int currentState;               // the current reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0; // the last time the output pin was toggled

void ICACHE_RAM_ATTR interrupt_routine()
{
    // If the switch/button changed, due to noise or pressing:
    if (currentState != lastFlickerableState)
    {
        // reset the debouncing timer
        lastDebounceTime = millis();
        // save the the last flickerable state
        lastFlickerableState = currentState;
    }

    if ((millis() - lastDebounceTime) > DEBOUNCE_TIME)
    {
        // whatever the reading is at, it's been there for longer than the debounce
        // delay, so take it as the actual current state:

        // if the button state has changed:
        if (lastSteadyState == HIGH && currentState == LOW)
        {
            Serial.println("The button is pressed");
            button_pressed = true;
        }
        else if (lastSteadyState == LOW && currentState == HIGH)
            Serial.println("The button is released");

        // save the the last steady state
        lastSteadyState = currentState;
    }
}

Property *propSwitch;
void invertRelay();

void setup()
{
    Serial.begin(115200);

    pinMode(REALY_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    digitalWrite(BUTTON_PIN, INPUT);

    digitalWrite(REALY_PIN, HIGH);

    attachInterrupt(BUTTON_PIN, interrupt_routine, CHANGE);

    // led
    digitalWrite(LED_PIN, LOW);

    Node *nodeSwitch = plat.NewNode("relay", "Lustr", NodeType::SWITCH);
    propSwitch = nodeSwitch->NewProperty("power", "Lustr", DataType::BOOLEAN);
    propSwitch->setSettable(true);
    propSwitch->setCallback([](Property *property)
                            {
                                Serial.printf("handling property value %s\n", property->getValue().c_str());

                                if (property->getValue() == "true")
                                    digitalWrite(REALY_PIN, HIGH);
                                else
                                    digitalWrite(REALY_PIN, LOW);
                            });

    auto toogleSwitch = nodeSwitch->NewProperty("toggle", "Vypínač", DataType::ENUM);
    toogleSwitch->setSettable(true);
    toogleSwitch->setFormat("on");
    toogleSwitch->setCallback([propSwitch](Property *property)
                              {
                                  Serial.printf("handling property value %s\n", property->getValue().c_str());

                                  invertRelay();
                              });
    // plat.clearTopic();
    // plat.saveTopic("/5d5554843beb3f419f9e4a64/garden/garage/gate");
    plat.enableOTA("123456777", 8266, "esp-vypinac01");
    plat.start();

    digitalWrite(LED_PIN, HIGH);
    propSwitch->setValue("true");
}

void loop()
{
    if (button_pressed)
    {
        invertRelay();
    }

    plat.loop();
}

void invertRelay()
{
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
}