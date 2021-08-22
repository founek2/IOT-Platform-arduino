#include <IOTPlatforma.h>
#include <WEMOS_SHT3X.h>
#include <Wire.h>

const char *ssid = "Zahrada"; //mqtt-test
const char *password = "1414elektron151";

const char *API_KEY = "41bc4e7fce86a08683ca01f2dd286350"; //test.iotplatforma.cloud - meteo

SHT3X sht30(0x45);

String keys[] = {"temp1", "hum", "volt"};

void setup()
{
    Serial.begin(115200);
    IOTPlatforma plat(API_KEY);
    plat.wifi(ssid, password);
    plat.enableOTA("123456777");
    plat.init();

    if (! sht31.begin(0x44)) 
  {
    Serial.println("Couldn't find SHT31");
  }

    sht30.get();

    float voltage = analogRead(A0) * 0.0047174;
    float val[] = {sht31.cTemp, sht31.humidity, voltage};
    plat.send(val, keys, 4);

    ESP.deepSleep(120e6);
}

void loop()
{
    // plat.loop();
}