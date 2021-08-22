#include <ESP8266WiFi.h>
#include "myconfig.h"
#include <IOTPlatform.h>

IOTPlatform plat("Anemometr");

unsigned long next_timestamp = 0;
volatile unsigned long i = 0;
float wind = 0;
float last_wind = 0;
int count = 0;
volatile unsigned long last_micros;
long debouncing_time = 5; //in millis
int input_pin = 13;
char charBuffer[32];

WiFiClient espClient;

void ICACHE_RAM_ATTR Interrupt()
{
  if ((long)(micros() - last_micros) >= debouncing_time * 1000)
  {
    i++;
    last_micros = micros();
    // Serial.println("otocka");
  }
}

const int sec_window = 15;

Property *propSpeed;
void setup()
{
  Serial.begin(115200);
  delay(10);
  pinMode(input_pin, INPUT_PULLUP); //D7
  pinMode(A0, INPUT);

  Node *node = plat.NewNode("anemometr", "Vítr", NodeType::SENSOR);
  propSpeed = node->NewProperty("wind", "Rychlost", DataType::FLOAT);
  propSpeed->setClass(PropertyClass::PRESSURE);
  propSpeed->setUnit("km/h");

  plat.enableOTA("123456777", 8266, "esp-anemo");
  plat.start();

  attachInterrupt(input_pin, Interrupt, RISING);
  // attachInterrupt(input_pin,Interrupt,RISING);
}

void loop()
{
  // Serial.print("rpm: ");
  // Serial.println(r);
  // delay(40);

  if (millis() > next_timestamp)
  {
    detachInterrupt(input_pin);
    count++;
    float rps = (i / number_reed) / sec_window; //computing rounds per second - every 3s
    if (i == 0)
      wind = 0.0;
    else
      wind = 1.761 / (1 + rps) + 3.013 * rps; // found here: https://www.amazon.de/gp/customer-reviews/R3C68WVOLJ7ZTO/ref=cm_cr_getr_d_rvw_ttl?ie=UTF8&ASIN=B0018LBFG8 (in German)
    if (last_wind - wind > 0.8 || last_wind - wind < -0.8 || count >= 10)
    {
      if (debugOutput)
      {
        Serial.print("Wind: ");
        Serial.print(wind);
        Serial.println(" km/h");
        Serial.print(wind / 3.6);
        Serial.println(" m/s");
      }

      float val[] = {wind};
      propSpeed->setValue(String(wind).c_str());

      count = 0;
    }
    i = 0;
    last_wind = wind;
    next_timestamp = millis() + sec_window * 1000; //sec into milices
    attachInterrupt(input_pin, Interrupt, RISING);
  }
  yield();
  plat.loop();
}