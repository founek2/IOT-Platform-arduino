#include "IOTPlatform.h"
#include "Arduino.h"

#define FASTLED_ESP8266_RAW_PIN_ORDER // The solution !!!!!!!!!!!!!!!!!!
#define LED_PIN 2                     // NODE MCU 14, wemos D4=2
#define NUM_LEDS 30 * 5
#define BRIGHTNESS 70
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

#define UPDATES_PER_SECOND 50

#include <FastLED.h>
CRGB leds[NUM_LEDS];

IOTPlatform plat("Postel");
Property *propSwitch;
bool change = false;

String currentColor = "103,58,183";
String currentType = "linear";
uint8_t currentBrightness = BRIGHTNESS;

// const int LED_PIN = D7; // doma
const int PIR_PIN = 13;

void fillWithRgb(String colorRgb);

void turnOnLed()
{
    Serial.println("on");
    FastLED.setBrightness(currentBrightness);
    FastLED.show();
    FastLED.delay(1000 / UPDATES_PER_SECOND);
    // propSwitch->setValue("true");
}

void turnOffLed()
{
    Serial.println("off");
    FastLED.setBrightness(0);
    FastLED.show();
    FastLED.delay(1000 / UPDATES_PER_SECOND);
    // propSwitch->setValue("false");
}

void ICACHE_RAM_ATTR interrupt_routine()
{
    change = true;
    Serial.println("interrupt");

    // int pir_state = digitalRead(PIR_PIN);
    // if (pir_state) turnOnLed();
    // else turnOffLed();
}

void setup()
{
    Serial.begin(115200);
    // plat.reset();
    pinMode(PIR_PIN, INPUT);
    attachInterrupt(PIR_PIN, interrupt_routine, CHANGE);

    int pir_state = digitalRead(PIR_PIN);

    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    fillWithRgb(currentColor);
    // FastLED.setBrightness(currentBrightness);
    if (pir_state)
        turnOnLed();
    else
        turnOffLed();

    Node *nodeSwitch = plat.NewNode("relay", "Postel", NodeType::SWITCH);
    propSwitch = nodeSwitch->NewProperty("power", "Postel led", DataType::BOOLEAN);
    propSwitch->setSettable(true);
    propSwitch->setCallback([](Property *property)
                            { 
                                if (property->getValue() == "true")
                                    turnOnLed();
                                else 
                                    turnOffLed();
                                return true; });
    Property *propColor = nodeSwitch->NewProperty("color", "Barva", DataType::COLOR);
    propColor->setFormat("rgb");
    propColor->setSettable(true);
    propColor->setRetainable(true);
    propColor->setCallback([](Property *prop)
                           {
                           currentColor = prop->getValue();
                           fillWithRgb(prop->getValue());
                           FastLED.show();

                           return true; });

    Property *propBright = nodeSwitch->NewProperty("bright", "Jas", DataType::INTEGER);
    propBright->setFormat("0:255");
    propBright->setSettable(true);
    propBright->setRetainable(true);
    propBright->setCallback([](Property *prop)
                            {
                            currentBrightness = prop->getValue().toInt();
                            FastLED.setBrightness(currentBrightness);
                            FastLED.show();
                            return true; });

    plat.enableOTA("123456777");

    propSwitch->setValue(pir_state ? "true" : "false");
}

int last_state = 0;

void loop()
{
    if (change)
    {
        change = false;
        Serial.println("change");

        int pir_state = digitalRead(PIR_PIN);
        if (pir_state)
            turnOnLed();
        else
            turnOffLed();

        propSwitch->setValue(pir_state ? "true" : "false");
    }

    // FastLED.show();
    // FastLED.delay(1000 / UPDATES_PER_SECOND);
    plat.loop();
}

void fillWithRgb(String colorRgb)
{
    int greenPos = colorRgb.indexOf(",") + 1;
    const int red = colorRgb.substring(0, greenPos - 1).toInt();
    int bluePos = colorRgb.indexOf(",", greenPos) + 1;
    const int green = colorRgb.substring(greenPos, bluePos - 1).toInt();
    int blue = colorRgb.substring(bluePos, colorRgb.length() - 1).toInt();

    Serial.println("rgb");
    Serial.println(red);
    Serial.println(green);
    Serial.println(blue);

    for (int i = 0; i < NUM_LEDS; i++)
    {
        leds[i].setRGB(red, green, blue);
    }
}