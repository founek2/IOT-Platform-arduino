#include "Seeed_BME280.h"
#include <IOTPlatforma.h>

const char *ssid = "OpenWrt"; //mqtt-test
const char *password = "trollmode";

const char * API_KEY = "7820f5a014c9b7b633e60be5c57b343e";
String topic = "/garden/weatherStation/save";

BME280 bme; // I2C
IOTPlatforma plat(API_KEY);

void setup()
{
      Serial.begin(115200);
      plat.wifi(ssid, password);

       if (!bme.init())
    {
        Serial.println("Device error!");
    }
}

void loop() {
    plat.loop();
    ESP.deepSleep(10e6);
}