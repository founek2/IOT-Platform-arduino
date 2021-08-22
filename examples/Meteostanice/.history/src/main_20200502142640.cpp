#include <IOTPlatforma.h>
#include <Adafruit_BMP280.h>

const char *ssid = "Zahrada"; //mqtt-test
const char *password = "1414elektron151";

const char *API_KEY = "41bc4e7fce86a08683ca01f2dd286350"; //test.iotplatforma.cloud - meteo

Adafruit_BMP280 bme;

String keys[] = {"temp1", "press", "hum", "volt"};

void setup()
{
    Serial.begin(115200);
    IOTPlatforma plat(API_KEY);
    plat.wifi(ssid, password);
    plat.enableOTA("123456777");
    plat.init();

    if (!bme.begin())
    {
        Serial.println("Device error!");
    }

    float voltage = analogRead(A0) * 0.0047174;
    float val[] = {bme.readTemperature(), (int)round(bme.readPressure() / 100.0), 0, voltage};
    plat.send(val, keys, 4);

    ESP.deepSleep(120e6);
}

void loop()
{
    // plat.loop();
}