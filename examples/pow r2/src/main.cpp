#include "IOTPlatform.h"
// #define SW_SERIAL_UNUSED_PIN -1
#include "CSE7766.h"
#include <Ticker.h>
#include <Queue.h>

IOTPlatform plat("Bouda", true);
Ticker tickerSend;
Ticker tickerMeasure;

bool RELAY_STATE = false;
bool reset_pressed = false;
const int REALY_PIN = 12;
const int LED_PIN = 13;
const int BUTTON_PIN = 0;
const int CSE7766_RX_PIN = 3;

CSE7766 myCSE7766;

bool READ_MEASUREMENTS = true;
bool SEND_MEASUREMENTS = false;

unsigned long buttonPressedAt = 0;
int volatile button_released = 1;
bool volatile resetDevice = false;
bool volatile quickButtonPress = false;

/* Calculate if should run another attempt
 * check milis() overflow, check: lastAttempt - now > interval */
bool shouldAttemptDebounce(unsigned long lastAttempt, unsigned long interval)
{
    auto now = millis();
    return lastAttempt > now || now - lastAttempt > interval;
}

void IRAM_ATTR interrupt_routine()
{
    button_released = digitalRead(BUTTON_PIN);

    if (button_released == LOW && buttonPressedAt == 0)
        buttonPressedAt = millis();
    else if (button_released == HIGH && buttonPressedAt != 0)
    {
        if (shouldAttemptDebounce(buttonPressedAt, 5 * 1000))
            resetDevice = true;
        else
            quickButtonPress = true;

        buttonPressedAt = 0;
    }
}

Property *propVoltage;
Property *propAmperate;
Property *propWattage;
Property *propReactive;
Property *propPowerFactor;
Property *propApparent;
Property *propSwitch;

void toogleRelay()
{
    if (propSwitch->getValue() == "true")
    {
        digitalWrite(REALY_PIN, LOW);
        propSwitch->setValue("false");
    }
    else
    {
        digitalWrite(REALY_PIN, HIGH);
        propSwitch->setValue("true");
    }
}
void setup()
{
    Serial.begin(115200);
    // plat.reset();
    pinMode(REALY_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(BUTTON_PIN, INPUT);
    digitalWrite(REALY_PIN, HIGH);

    attachInterrupt(BUTTON_PIN, interrupt_routine, CHANGE);

    // led
    digitalWrite(LED_PIN, LOW);

    Node *nodeSwitch = plat.NewNode("relay", "Spínač", NodeType::SWITCH);
    propSwitch = nodeSwitch->NewProperty("power", "Spínač", DataType::BOOLEAN);
    propSwitch->setSettable(true);
    propSwitch->setCallback([](Property *property)
                            {
                                Serial.printf("handling property value %s\n", property->getValue().c_str());

                                if (property->getValue() == "true")
                                    digitalWrite(REALY_PIN, HIGH);
                                else
                                    digitalWrite(REALY_PIN, LOW);
                                return true; });

    auto toogleSwitch = nodeSwitch->NewProperty("toggle", "toogle", DataType::ENUM);
    toogleSwitch->setSettable(true);
    toogleSwitch->setFormat("on");
    toogleSwitch->setCallback([](Property *property)
                              {
                                  Serial.printf("handling property value %s\n", property->getValue().c_str());

                                  toogleRelay();
                                  return true; });

    Node *nodeMeasurement = plat.NewNode("measurement", "Spotřeba", NodeType::SENSOR);
    propWattage = nodeMeasurement->NewProperty("wattage", "Spotřeba", DataType::FLOAT);
    propWattage->setClass(PropertyClass::VOLTAGE);
    propWattage->setUnit("W");

    propAmperate = nodeMeasurement->NewProperty("amperage", "Proud", DataType::FLOAT);
    propAmperate->setUnit("A");

    propVoltage = nodeMeasurement->NewProperty("voltage", "Napětí", DataType::FLOAT);
    propVoltage->setClass(PropertyClass::VOLTAGE);
    propVoltage->setUnit("V");

    propApparent = nodeMeasurement->NewProperty("apparent", "Zdánlivá síla", DataType::FLOAT);
    propApparent->setUnit("VA");

    propReactive = nodeMeasurement->NewProperty("reactive", "Reaktivní síla", DataType::FLOAT);
    propReactive->setUnit("var");

    propPowerFactor = nodeMeasurement->NewProperty("factor", "Faktor síly", DataType::FLOAT);

    // plat.clearTopic();
    // plat.saveTopic("/5d5554843beb3f419f9e4a64/garden/garage/gate");
    plat.enableOTA("123456777", 8266, "esp-pow-r2");

    // myCSE7766.setCurrentRatio(CSE7766_CURRENT_RATIO);
    // myCSE7766.setVoltageRatio(CSE7766_VOLTAGE_RATIO);
    // myCSE7766.setPowerRatio(CSE7766_POWER_RATIO);
    // myCSE7766.setRX(CSE7766_RX_PIN);
    myCSE7766.begin(); // will initialize serial to 4800 bps

    digitalWrite(LED_PIN, HIGH);
    propSwitch->setValue("true");

    tickerMeasure.attach(5, []()
                         { READ_MEASUREMENTS = true; });
    tickerSend.attach(30, []()
                      { SEND_MEASUREMENTS = true; });
}

const int q_size = 7;
Queue<double, q_size> queue;

void addWattage(double &value)
{
    if (queue.size() >= q_size)
    {
        queue.dequeue();
    }
    queue.enqueue(value);
}

double calcWattageMean()
{
    double tmp = 0;
    for (int i = 0; i < queue.size(); i++)
    {
        tmp += queue.at(i);
    }

    return tmp / queue.size();
}

void loop()
{
    plat.loop();

    if (quickButtonPress)
    {
        quickButtonPress = false;
        toogleRelay();
    }

    if (resetDevice)
    {
        Serial.println("device reset");
        plat.disconnect();
        plat.reset();
        digitalWrite(LED_PIN, LOW);
        delay(500);
        ESP.restart();
    }

    if (READ_MEASUREMENTS == true)
    {
        READ_MEASUREMENTS = false;

        myCSE7766.handle();

        auto value = myCSE7766.getActivePower();
        addWattage(value);
    }

    if (SEND_MEASUREMENTS == true)
    {
        SEND_MEASUREMENTS = false;

        // auto value = propWattage->setValue(String(myCSE7766.getActivePower()).c_str());
        propWattage->setValue(String(calcWattageMean()).c_str());

        propReactive->setValue(String(myCSE7766.getReactivePower()).c_str());
        propPowerFactor->setValue(String(myCSE7766.getPowerFactor()).c_str());
        propApparent->setValue(String(myCSE7766.getApparentPower()).c_str());
        propVoltage->setValue(String(myCSE7766.getVoltage()).c_str());
        propAmperate->setValue(String(myCSE7766.getCurrent()).c_str());
    }
}