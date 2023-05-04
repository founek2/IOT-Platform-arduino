#include "IOTPlatform.h"
#include <DallasTemperature.h>
#include <Ticker.h>

#define ONE_WIRE_BUS 25

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensor(&oneWire);

Ticker tempTicket;

IOTPlatform plat("Bojler stykač", true);
Property *propSwitch;
Property *propTariff;
Property *propTemp;

bool RELAY_STATE = false;
bool reset_pressed = false;
int REALY_PIN1 = 19;
int REALY_PIN2 = 22;
int LED_POWER_PIN = 16;
int LED_APP_PIN = 13;  // APP led - LOW = light
int LED_WIFI_PIN = 15; // APP led - LOW = light
int BUTTON_PIN = 0;
int TARIFF_PIN = 5;
int SENSOR_POWER_PIN = 27;

bool SEND_TEMP_ENABLED = true;

const int LEVNY_TARIF = LOW;
const int DRAHY_TARIF = HIGH;
int tariffStatus = DRAHY_TARIF; // HIGH = expensive, LOW = cheap
unsigned long lastReadAttempt = 0;
const unsigned long READ_INTERVAL = 10 * 1000;

unsigned long volatile buttonPressedAt = 0;
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

bool status = false;
void switchRelay(bool newStatus)
{
    if (status == newStatus)
        return;

    status = newStatus;
    if (status)
    {

        digitalWrite(LED_POWER_PIN, LOW);
        digitalWrite(REALY_PIN1, LOW);
        digitalWrite(REALY_PIN2, HIGH);
    }
    else
    {
        digitalWrite(LED_POWER_PIN, HIGH);
        digitalWrite(REALY_PIN1, HIGH);
        digitalWrite(REALY_PIN2, LOW);
    }
}
bool handleRelay(Property *property)
{
    Serial.printf("handling property value %s\n", property->getValue().c_str());

    if (property->getValue() == "true")
        switchRelay(true);
    else
        switchRelay(false);

    return true;
};

void sendTemp()
{
    tempSensor.requestTemperatures();
    float temp = tempSensor.getTempCByIndex(0);
    if (temp != DEVICE_DISCONNECTED_C)
    {
        char array[10];
        sprintf(array, "%.2f", temp);
        propTemp->setValue(array);
    }
}

void setup()
{
    Serial.begin(115200);
    // plat.reset();
    pinMode(REALY_PIN1, OUTPUT);
    pinMode(REALY_PIN2, OUTPUT);
    pinMode(LED_POWER_PIN, OUTPUT);
    pinMode(LED_APP_PIN, OUTPUT);
    pinMode(LED_WIFI_PIN, OUTPUT);
    pinMode(SENSOR_POWER_PIN, OUTPUT);
    pinMode(TARIFF_PIN, INPUT_PULLUP);

    // vypnuto
    digitalWrite(REALY_PIN1, HIGH);
    digitalWrite(REALY_PIN2, LOW);

    // power up temperature sensor
    digitalWrite(SENSOR_POWER_PIN, HIGH);

    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), interrupt_routine, CHANGE);

    // led
    digitalWrite(LED_APP_PIN, HIGH);
    digitalWrite(LED_WIFI_PIN, LOW);
    digitalWrite(LED_POWER_PIN, HIGH);

    Node *nodeSwitch = plat.NewNode("relay", "Bojler", NodeType::SWITCH);
    propSwitch = nodeSwitch->NewProperty("power", "Bojler", DataType::BOOLEAN);
    propSwitch->setSettable(true);
    propSwitch->setCallback(handleRelay);

    propTariff = nodeSwitch->NewProperty("tariff", "Levný tarif", DataType::ENUM);
    propTariff->setFormat("ano,ne");

    propTemp = nodeSwitch->NewProperty("temp", "Teplota", DataType::FLOAT);
    propTemp->setClass(PropertyClass::TEMPERATURE);
    propTariff->setUnit("°C");

    plat.enableOTA("123456777", 8266, "esp-bojler");

    propSwitch->setValue("false");
    propTariff->setValue("ne");

    tempSensor.begin();

    tempTicket.attach(5 * 60, []()
                      { SEND_TEMP_ENABLED = true; });
}

void loop()
{
    // Turn APP_LED led during button press
    if (button_released)
        digitalWrite(LED_APP_PIN, HIGH);
    else
        digitalWrite(LED_APP_PIN, LOW);

    // Turn WIFI led when disconnected
    if (plat.state() == ConnectionStatus::CONNECTED)
        digitalWrite(LED_WIFI_PIN, HIGH);
    else if (plat.state() == ConnectionStatus::DISCONNECTED)
        digitalWrite(LED_WIFI_PIN, LOW);

    if (resetDevice)
    {
        Serial.println("device reset");
        plat.disconnect();
        plat.reset();
        digitalWrite(LED_APP_PIN, LOW);
        digitalWrite(LED_WIFI_PIN, LOW);
        digitalWrite(LED_POWER_PIN, LOW);
        delay(500);
        ESP.restart();
    }
    else if (quickButtonPress)
    {
        quickButtonPress = false;
        Serial.println("button press");

        bool newStatues = !status;
        if (newStatues)
            propSwitch->setValue("true");
        else
            propSwitch->setValue("false");

        switchRelay(newStatues);
    }

    if (shouldAttemptDebounce(lastReadAttempt, READ_INTERVAL))
    {
        lastReadAttempt = millis();

        int currentTariffStatus = digitalRead(TARIFF_PIN);
        if (currentTariffStatus != tariffStatus)
        {
            tariffStatus = currentTariffStatus;
            if (plat.state() == ConnectionStatus::CONNECTED)
            {
                if (tariffStatus == DRAHY_TARIF)
                    propTariff->setValue("ne");
                else
                    propTariff->setValue("ano");
            }

            // When disconnected driven by tarrif status
            if (plat.state() != ConnectionStatus::CONNECTED)
            {
                if (tariffStatus == DRAHY_TARIF)
                {
                    switchRelay(false);
                    propTariff->setValue("ne");
                    propSwitch->setValue("false");
                }
                else
                {
                    switchRelay(true);
                    propTariff->setValue("ano");
                    propSwitch->setValue("true");
                }
            }
        }
    }

    if (SEND_TEMP_ENABLED)
    {
        sendTemp();
        SEND_TEMP_ENABLED = false;
    }

    plat.loop();
}