#include <IOTPlatforma.h>
#include "Adafruit_SHT31.h"
#include <Wire.h>

const char *ssid = "Zahrada"; //mqtt-test
const char *password = "1414elektron151";

const char *API_KEY = "41bc4e7fce86a08683ca01f2dd286350"; //test.iotplatforma.cloud - meteo

Adafruit_SHT31 sht31 = Adafruit_SHT31();

String keys[] = {"temp1", "hum", "volt"};

void setup()
{
    Serial.begin(115200);
    IOTPlatforma plat(API_KEY);
    plat.wifi(ssid, password);
    plat.enableOTA("123456777");
    plat.init();

  if (!sht31.begin(0x44))
  {
    Serial.println("Couldn't find SHT31");
  }

    float voltage = analogRead(A0) * 0.0047174;
    float val[] = {sht31.readTemperature(), sht31.readHumidity(), voltage};
    plat.send(val, keys, 4);

    ESP.deepSleep(120e6);
}

void loop()
{
    // plat.loop();
}