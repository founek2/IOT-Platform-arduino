#include <Arduino.h>
#include "IOTPlatform.h"

#define FASTLED_ESP8266_RAW_PIN_ORDER // The solution !!!!!!!!!!!!!!!!!!
#define LED_PIN 2                     // NODE MCU 14, wemos D4=2
#define NUM_LEDS 60 * 4
#define BRIGHTNESS 100
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

#include <FastLED.h>
CRGB leds[NUM_LEDS];

#define UPDATES_PER_SECOND 50

IOTPlatform plat("Led RGB");

CRGBPalette16 currentPalette;
TBlendType currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

void FillLEDsFromPaletteColors(uint8_t colorIndex);
long getHex(String color);
void FillLEDs(long color);
void Fire2012WithPalette();
void copyFirstHalfOfLEDs();

// CRGBPalette16 tricolorPallet = CRGBPalette16( CRGB::Red, CRGB::White, CRGB::Blue);

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

String currentColor = "103,58,183";
String currentType = "linear";
uint8_t currentBrightness = BRIGHTNESS;

bool gReverseDirection = false;

void setup()
{
  Serial.begin(115200);
  // plat.reset();
  currentPalette = CRGBPalette16(CRGB::Red, CRGB::White, CRGB::Blue);
  currentBlending = NOBLEND;

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(currentBrightness);

  fillWithRgb(currentColor);
  FastLED.show();
  FastLED.delay(10);

  Node *node = plat.NewNode("led", "Led RGB", NodeType::SWITCH);
  Property *propPower = node->NewProperty("power", "Zapnuto", DataType::BOOLEAN);
  propPower->setSettable(true);
  propPower->setCallback([](Property *prop)
                         {
                           if (prop->getValue() == "false")
                           {
                             FastLED.clear(); // clear all pixel data
                             FastLED.show();
                           }
                           else
                           {
                             fillWithRgb(currentColor);
                             FastLED.show();
                           } });

  Property *propColor = node->NewProperty("color", "Barva", DataType::COLOR);
  propColor->setFormat("rgb");
  propColor->setSettable(true);
  propColor->setRetainable(true);
  propColor->setCallback([](Property *prop)
                         {
                           currentColor = prop->getValue();
                           fillWithRgb(prop->getValue());
                           FastLED.show();
                           ; });

  Property *propBright = node->NewProperty("bright", "Jas", DataType::INTEGER);
  propBright->setFormat("0:255");
  propBright->setSettable(true);
  propBright->setRetainable(true);
  propBright->setCallback([](Property *prop)
                          {
                            currentBrightness = prop->getValue().toInt();
                            FastLED.setBrightness(currentBrightness);
                            FastLED.show();
                            ; });

  Property *propType = node->NewProperty("type", "Typ", DataType::ENUM);
  propType->setFormat("linear,fire,tricolor");
  propType->setSettable(true);
  propType->setRetainable(true);
  propType->setCallback([](Property *prop)
                        {
                            currentType = prop->getValue();

                            if (prop->getValue() == "linear") {
                              fillWithRgb(prop->getValue());
                              FastLED.setBrightness(currentBrightness);
                              FastLED.show();  

                            } });

  plat.enableOTA("123456777", 8266, "esp-rgb");
  plat.start();

  propPower->setValue("true");
}

uint8_t startIndex = 0;
void loop()
{

  if (currentType == "fire")
  {
    Fire2012WithPalette(); // run simulation frame
    copyFirstHalfOfLEDs(); // mirror to
  }
  else if (currentType == "tricolor")
  {
    startIndex = startIndex + 1; /* motion speed */

    FillLEDsFromPaletteColors(startIndex);
  }

  FastLED.show();
  FastLED.delay(1000 / UPDATES_PER_SECOND);

  plat.loop();
}

void FillLEDsFromPaletteColors(uint8_t colorIndex)
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = ColorFromPalette(currentPalette, colorIndex, currentBrightness, currentBlending);
    colorIndex += 2;
  }
}

void FillLEDs(long color)
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = color;
  }
}

long getHex(String color)
{
  String colorString = color.substring(2); // remove Q:
  return strtol(colorString.c_str(), NULL, 16);
}

void copyFirstHalfOfLEDs()
{
  for (int i = 0; i < NUM_LEDS / 2; i++)
  {
    leds[NUM_LEDS - 1 - i] = leds[i];
  }
}

// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100
#define COOLING 15

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 120

CRGBPalette16 gPal = HeatColors_p;

void Fire2012WithPalette()
{
  // Array of temperature readings at each simulation cell
  static uint8_t heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
  for (int i = 0; i < NUM_LEDS; i++)
  {
    heat[i] = qsub8(heat[i], random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for (int k = NUM_LEDS - 1; k >= 2; k--)
  {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if (random8() < SPARKING)
  {
    int y = random8(7);
    heat[y] = qadd8(heat[y], random8(160, 255));
  }

  // Step 4.  Map from heat cells to LED colors
  for (int j = 0; j < NUM_LEDS; j++)
  {
    // Scale the heat value from 0-255 down to 0-240
    // for best results with color palettes.
    uint8_t colorindex = scale8(heat[j], 240);
    CRGB color = ColorFromPalette(gPal, colorindex);
    int pixelnumber;
    if (gReverseDirection)
    {
      pixelnumber = (NUM_LEDS - 1) - j;
    }
    else
    {
      pixelnumber = j;
    }
    leds[pixelnumber] = color;
  }
}
