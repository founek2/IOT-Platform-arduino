#include <IOTPlatform.h>
#include <Wire.h>
#include "Seeed_BME280.h"
#include <Ticker.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS D6

BME280 bme280;
IOTPlatform plat("Meteostanice");
Ticker ticker;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensor(&oneWire);

Property *propTemp;
Property *propTemp2;
Property *propPressure;
Property *propVoltage;
Property *propHumidity;
bool SEND_MEASUREMENTS = true;

void sendMeasurements();

void setup()
{
    Serial.begin(115200);
    // plat.reset();

    Node *node = plat.NewNode("meteo", "Meteo", NodeType::SENSOR);
    propTemp = node->NewProperty("temp", "Teplota", DataType::FLOAT);
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

    propTemp2 = node->NewProperty("temp2", "Teplota2", DataType::FLOAT);
    propTemp2->setClass(PropertyClass::TEMPERATURE);
    propTemp2->setUnit("°C");

    if (!bme280.init())
    {
        Serial.println("Device error!");
    }
    tempSensor.begin();

    plat.enableOTA("123456777", 8266, "esp-meteo");
    plat.start();

    ticker.attach(15 * 60, []()
                  { SEND_MEASUREMENTS = true; });

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
    propTemp2->setValue(String(bme280.getTemperature(), 1).c_str());
    propHumidity->setValue(String((int)bme280.getHumidity(), DEC).c_str());
    propPressure->setValue(String(bme280.getPressure() / 100).c_str());
    propVoltage->setValue(String(voltage, 1).c_str());

    tempSensor.requestTemperatures();
    propTemp->setValue(String(tempSensor.getTempCByIndex(0), 1).c_str());
}