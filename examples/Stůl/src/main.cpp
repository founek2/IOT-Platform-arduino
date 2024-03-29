#include "IOTPlatform.h"
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <IRsend.h>
#include <Ticker.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Yamaha.h"

#define ONE_WIRE_BUS D1

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensor(&oneWire);

Ticker blinker;
Ticker tempTicket;
Ticker yamahaTicket;
IOTPlatform plat("Stůl");
Yamaha yamaha("192.168.10.238");

void checkIrReciever();
void sendTemp();
void processStates();
void syncYamaha();

bool RELAY1_STATE = false;
bool RELAY2_STATE = false;
int REALY_PIN1 = 14;
int REALY_PIN2 = 12;
uint16_t IR_RECIEVER_PIN = 2;
uint16_t IR_LED_PIN = D3;
uint64_t poweroff_data = 0x40BDA25D;
uint64_t num3_data = 0x40BD40BF;

IRrecv irrecv(IR_RECIEVER_PIN);
IRsend irsend(D2); // Set the GPIO to be used to sending the message
decode_results results;

bool SEND_TEMP_ENABLED = true;
bool CHECK_RECIEVER_ENABLED = true;
bool CHECK_YAMAHA_ENABLED = false;

Property *propTemp;
Property *switchL;
Property *switchR;
Property *propSwitch;

Property *recieverPower;
Property *recieverVolume;
Property *recieverInput;

bool recieverChange = false;

void setup()
{
    Serial.begin(115200);
    pinMode(REALY_PIN1, OUTPUT);
    pinMode(REALY_PIN2, OUTPUT);
    pinMode(13, OUTPUT);
    // plat.reset();
    irsend.begin();
    irrecv.enableIRIn(); // Start the receiver

    Node *nodeSwitch = plat.NewNode("relay", "Led stůl", NodeType::SWITCH);
    propSwitch = nodeSwitch->NewProperty("power", "Led stůl", DataType::BOOLEAN);
    propSwitch->setSettable(true);
    propSwitch->setCallback([](Property *property)
                            {
                                Serial.printf("handling property value %s\n", property->getValue().c_str());

                                if (property->getValue() == "true")
                                    RELAY1_STATE = RELAY2_STATE = true;
                                else
                                    RELAY1_STATE = RELAY2_STATE = false;

                                processStates();

                                switchL->setValue(RELAY1_STATE ? "true" : "false");
                                switchR->setValue(RELAY2_STATE ? "true" : "false");
                                return true; });

    switchL = nodeSwitch->NewProperty("left", "Levé", DataType::BOOLEAN);
    switchL->setSettable(true);
    switchL->setCallback([](Property *property)
                         {
                             Serial.printf("handling property value %s\n", property->getValue().c_str());

                             if (property->getValue() == "true")
                                 RELAY1_STATE = true;
                             else
                                 RELAY1_STATE = false;

                             processStates();
                             return true; });

    switchR = nodeSwitch->NewProperty("right", "Pravé", DataType::BOOLEAN);
    switchR->setSettable(true);
    switchR->setCallback([](Property *property)
                         {
                             Serial.printf("handling property value %s\n", property->getValue().c_str());

                             if (property->getValue() == "true")
                                 RELAY2_STATE = true;
                             else
                                 RELAY2_STATE = false;

                             processStates();
                             return true; });

    Node *temp = plat.NewNode("sensor0", "Teplota", NodeType::SENSOR);
    propTemp = temp->NewProperty("temperature", "Teplota", DataType::FLOAT);
    propTemp->setClass(PropertyClass::TEMPERATURE);
    propTemp->setUnit("°C");

    Node *reciever = plat.NewNode("reciever", "Reciever", NodeType::SWITCH);
    recieverPower = reciever->NewProperty("power", "Zapnutí", DataType::BOOLEAN);
    recieverPower->setSettable(true);
    recieverPower->setCallback([](Property *prop)
                               { recieverChange = true;
                               return true; });

    recieverVolume = reciever->NewProperty("volume", "Hlasitost", DataType::INTEGER);
    recieverVolume->setFormat("0:80");
    recieverVolume->setSettable(true);
    recieverVolume->setCallback([](Property *prop)
                                { recieverChange = true;
                                return true; });

    recieverInput = reciever->NewProperty("input", "Vstup", DataType::ENUM);
    recieverInput->setFormat("net_radio,bluetooth,airplay,tuner,spotify");
    recieverInput->setSettable(true);
    recieverInput->setCallback([](Property *prop)
                               { recieverChange = true;
                               return true; });

    tempSensor.begin();

    blinker.attach(0.2, []()
                   { CHECK_RECIEVER_ENABLED = true; });
    tempTicket.attach(10 * 60, []()
                      { SEND_TEMP_ENABLED = true; });
    yamahaTicket.attach(5 * 60, []()
                        { CHECK_YAMAHA_ENABLED = true; });

    switchL->setValue("false");
    switchR->setValue("false");
    propSwitch->setValue("false");
}

void loop()
{
    plat.loop();
    if (SEND_TEMP_ENABLED)
    {
        sendTemp();
        SEND_TEMP_ENABLED = false;
    }
    if (CHECK_RECIEVER_ENABLED)
    {
        checkIrReciever();
        CHECK_RECIEVER_ENABLED = false;
    }
    if (CHECK_YAMAHA_ENABLED)
    {
        if (yamaha.checkStatus())
            syncYamaha();
        CHECK_YAMAHA_ENABLED = false;
    }

    if (recieverChange)
    {
        recieverChange = false;
        if (recieverPower->getValue() != yamaha.getPower())
        {
            if (recieverPower->getValue() == "true")
                yamaha.powerOn();
            else
                yamaha.powerOff();
        }

        if (recieverInput->getValue() != yamaha.getInput())
            yamaha.setInput(recieverInput->getValue().c_str());

        if (recieverVolume->getValue().toInt() != yamaha.getVolume())
            yamaha.setVolume(recieverVolume->getValue().toInt());
    }
}

void syncYamaha()
{
    Serial.printf("sync power=%s volume=%d input=%s\n", yamaha.getPower().c_str(), yamaha.getVolume(), yamaha.getInput().c_str());
    if (recieverPower->getValue() != yamaha.getPower())
        recieverPower->setValue(yamaha.getPower().c_str());
    if (recieverVolume->getValue() != String(yamaha.getVolume()))
        recieverVolume->setValue(String(yamaha.getVolume()).c_str());
    if (recieverInput->getValue() != yamaha.getInput())
        recieverInput->setValue(yamaha.getInput().c_str());
}

void setPin(boolean state, int pin)
{
    digitalWrite(pin, state);
    Serial.println(pin);
}

void processStates()
{
    if (RELAY1_STATE)
    {
        Serial.println("relay1 true");
        setPin(HIGH, REALY_PIN2);
    }
    else
    {
        Serial.println("relay1 false");
        setPin(LOW, REALY_PIN2);
    }

    if (RELAY2_STATE)
    {
        setPin(HIGH, REALY_PIN1);
        RELAY2_STATE = true;
        Serial.println("relay2 false");
    }
    else
    {
        Serial.println("relay2 true");
        setPin(LOW, REALY_PIN1);
        RELAY2_STATE = false;
    }

    if ((RELAY1_STATE || RELAY2_STATE) && propSwitch->getValue() == "false")
        propSwitch->setValue("true");
    else if ((!RELAY1_STATE && !RELAY2_STATE) && propSwitch->getValue() == "true")
        propSwitch->setValue("false");
}

const String NUM_1_HEX = "40BD00FF"; // 1
const String NUM_2_HEX = "40BD807F"; // 2
const String NUM_3_HEX = "40BD40BF";
const String POWER_HEX = "40BDA25D";
const String UP_HEX = "40BD48B7";
const String DOWN_HEX = "40BDC837";
const String LEFT_HEX = "40BD8877";
const String RIGHT_HEX = "40BD08F7";
const String ESC_HEX = "40BD38C7";
const String VOLUME_DOWN_HEX = "40BD708F";
const String STOP_HEX = "40BD9867";
const String RECORD_HEX = "40BD9867";
const String SHUFFLE_HEX = "40BD22DD";

const uint64_t UP_BOX_DATA = 0x80BF53AC;
const uint64_t DOWN_BOX_DATA = 0x80BF4BB4;
const uint64_t RIGHT_BOX_DATA = 0x80BF837C;
const uint64_t LEFT_BOX_DATA = 0x80BF9966;
const uint64_t POWER_TV_DATA = 0x20DF10EF;
const uint64_t POWER_BOX_DATA = 0x80BF3BC4;
const uint64_t VOLUME_DOWN_TV_DATA = 0x20DFC03F;
const uint64_t AV_MODE_DATA = 0x20DF0CF3;
const uint64_t NEGATE_TV_LED = 0x23333333;

void checkIrReciever()
{
    if (irrecv.decode(&results))
    {
        // print() & println() can't handle printing long longs. (uint64_t)
        String hex = uint64ToString(results.value, HEX);
        Serial.println("hex " + hex);
        Serial.print(resultToHumanReadableBasic(&results));

        // Serial.println(resultToSourceCode(&results));
        // Serial.println(results.command);
        Serial.println("");

        if (NUM_1_HEX == hex)
        {
            RELAY1_STATE = !RELAY1_STATE;
            switchL->setValue(RELAY1_STATE ? "true" : "false");
            Serial.println("left");
            processStates();
        }
        else if (NUM_2_HEX == hex)
        {
            RELAY2_STATE = !RELAY2_STATE;
            switchR->setValue(RELAY2_STATE ? "true" : "false");
            processStates();
        }
        else if (NUM_3_HEX == hex)
        {
            Serial.println("num3");
            irsend.sendNEC(num3_data);
        }
        else if (UP_HEX == hex)
        {
            Serial.println("UP");
            irsend.sendNEC(UP_BOX_DATA);
        }
        else if (DOWN_HEX == hex)
        {
            Serial.println("DOWN");
            irsend.sendNEC(DOWN_BOX_DATA);
        }
        else if (POWER_HEX == hex)
        {
            plat.publish("v2/martas/ESP-A754D4/relay/toggle/set", "on");
            // Serial.println("sending power lustr");
            // SEND_LUSTR_CHANGE = true;
        }
        else if (LEFT_HEX == hex)
        {
            Serial.println("left");
            irsend.sendNEC(LEFT_BOX_DATA);
        }
        else if (RIGHT_HEX == hex)
        {
            Serial.println("right");
            irsend.sendNEC(RIGHT_BOX_DATA);
        }
        else if (RECORD_HEX == hex)
        {
            Serial.println("box power");
            irsend.sendNEC(POWER_BOX_DATA);
        }
        else if (STOP_HEX == hex)
        {
            Serial.println("power tv");
            irsend.sendNEC(POWER_TV_DATA);
        }
        else if (VOLUME_DOWN_HEX == hex)
        {
            Serial.println("volume down tv");
            irsend.sendNEC(AV_MODE_DATA, 32);
        }
        else if (SHUFFLE_HEX == hex)
        {
            Serial.println("turn LED tv");
            irsend.sendNEC(NEGATE_TV_LED, 32);
        }

        irrecv.resume(); // Receive the next value
    }
}

void sendTemp()
{
    tempSensor.requestTemperatures();
    float temp = tempSensor.getTempCByIndex(0);
    if (temp > -100)
    {
        char array[10];
        sprintf(array, "%.2f", temp);
        propTemp->setValue(array);
    }
}