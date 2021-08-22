#include <ESP8266WiFi.h>
#include "myconfig.h"
#include <Wire.h>
#include "Adafruit_SHT31.h"
#include <IOTPlatforma.h>

const char *API_KEY = "41bc4e7fce86a08683ca01f2dd286350"; //v2.iotplatforma.cloud - meteo
const char *ssid = "Zahrada";                             //mqtt-test
const char *password = "1414elektron151";
String keys[] = {"temp1", "hum", "wind"};

Adafruit_SHT31 sht31 = Adafruit_SHT31();

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

IOTPlatforma plat(API_KEY);

void setup()
{
  Serial.begin(115200);
  delay(10);
  pinMode(input_pin, INPUT_PULLUP); //D7
  pinMode(A0, INPUT);
  // We start by connecting to a WiFi network
  plat.wifi(ssid, password);
  plat.enableOTA("123456777");
  plat.init();
  //   do_update();
  //   client.setServer(mqtt_host, mqtt_port);
  //   reconnect();
  attachInterrupt(input_pin, Interrupt, RISING);
  // attachInterrupt(input_pin,Interrupt,RISING);
  if (!sht31.begin(0x44))
  {
    Serial.println("Couldn't find SHT31");
    while (1)
      delay(1);
  }
}

void loop()
{
  int r = digitalRead(input_pin);
  // Serial.print("rpm: ");
  // Serial.println(r);
  // delay(40);

  if (millis() > next_timestamp)
  {
    detachInterrupt(input_pin);
    count++;
    float rps = i / number_reed; //computing rounds per second
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
      plat.send(val, keys, 3);

      String strBuffer;
      strBuffer = String(wind);
      strBuffer.toCharArray(charBuffer, 10);
      count = 0;
    }
    i = 0;
    last_wind = wind;
    next_timestamp = millis() + 1000; //intervall is 1s
    attachInterrupt(input_pin, Interrupt, RISING);
  }
  yield();
}