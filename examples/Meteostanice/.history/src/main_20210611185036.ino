#include <IOTPlatforma.h>
#include <Wire.h>
#include "Seeed_BME280.h"
#include <Ticker.h>

void sendMeasurements();

BME280 bme280;
IOTPlatforma plat("Meteostanice");
Ticker ticker;

Property *propTemp;
Property *propPressure;
Property *propVoltage;
Property *propHumidity;
bool SEND_MEASUREMENTS = true;

void setup()
{
    Serial.begin(115200);

    Node *node = plat.NewNode("meteo", "Meteo", NodeType::SENSOR);
    propTemp = node->NewProperty("temperature", "Teplota", DataType::FLOAT);
    propTemp->setClass(PropertyClass::TEMPERATURE);
    propTemp->setUnit("°C");

    propHumidity = node->NewProperty("humidity", "Vlhkost", DataType::INTEGER);
    propHumidity->setClass(PropertyClass::HUMIDITY);
    propHumidity->setUnit("%");

    propPressure = node->NewProperty("pressure", "Tlak", DataType::INTEGER);
    propPressure->setClass(PropertyClass::PRESSURE);
    propPressure->setUnit("hPa");

    propVoltage = node->NewProperty("voltage", "Napětí", DataType::FLOAT);
    propVoltage->setClass(PropertyClass::VOLTAGE);
    propVoltage->setUnit("V");

    if (!bme280.init())
    {
        Serial.println("Device error!");
    }

    plat.enableOTA("123456777");
    plat.start();

    ticker.attach(15 * 60, []() { SEND_MEASUREMENTS = true; });

    // plat.sleep();
    // ESP.deepSleep(15 * 60 * 1000000); // deepSleep time is defined in microseconds.
}

void loop()
{
    plat.loop();

    if (SEND_MEASUREMENTS)
    {
        sendMeasurements();
        SEND_MEASUREMENTS = false;
    }
}

void sendMeasurements()
{
    float voltage = analogRead(A0) * 0.0047174;
    propTemp->setValue(String(bme280.getTemperature(), 1).c_str());
    propHumidity->setValue(String((int)bme280.getHumidity(), DEC).c_str());
    propPressure->setValue(String(bme280.getPressure() / 100).c_str());
    propVoltage->setValue(String(voltage, 1).c_str());
}