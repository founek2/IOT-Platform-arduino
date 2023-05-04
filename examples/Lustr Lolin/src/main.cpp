#include "IOTPlatform.h"

// const char *ssid = "MujO2Internet_2.4G_C9A2B9";
// const char *password = "83902697";

IOTPlatform plat("Lustr", true);

bool RELAY_STATE = false;
bool reset_pressed = false;
int REALY_PIN = 12;
int LED_PIN = 2;
int BUTTON_PIN = 0;

unsigned long buttonPressedAt = 0;
int volatile button_released = 1;
bool volatile resetDevice = false;
bool volatile quickButtonPress = false;

/* Calculate if should run another attempt
 * check milis() overflow, check: lastAttempt - now > interval */
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

void setup()
{
    Serial.begin(115200);
    Serial.println("booting up");
    // plat.reset();
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
    propSwitch->setCallback([](Property *property)
                            {
                                Serial.printf("handling property value %s\n", property->getValue().c_str());

                                if (property->getValue() == "true")
                                    digitalWrite(REALY_PIN, HIGH);
                                else
                                    digitalWrite(REALY_PIN, LOW);
                                
                                return true; });

    auto toogleSwitch = nodeSwitch->NewProperty("toggle", "toogle", DataType::ENUM);
    toogleSwitch->setSettable(true);
    toogleSwitch->setFormat("on");
    toogleSwitch->setCallback([propSwitch](Property *property)
                              {
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
                                  return true; });
    // plat.clearTopic();
    // plat.saveTopic("/5d5554843beb3f419f9e4a64/garden/garage/gate");
    plat.enableOTA("123456777", 8266, "esp-lustr");

    digitalWrite(LED_PIN, HIGH);
    propSwitch->setValue("true");
}

void loop()
{
    // Turn APP_LED led during button press
    if (button_released)
        digitalWrite(LED_PIN, HIGH);
    else
        digitalWrite(LED_PIN, LOW);

    if (resetDevice)
    {
        Serial.println("device reset");
        plat.disconnect();
        plat.reset();
        digitalWrite(LED_PIN, LOW);
        delay(500);
        ESP.restart();
    }

    plat.loop();
}