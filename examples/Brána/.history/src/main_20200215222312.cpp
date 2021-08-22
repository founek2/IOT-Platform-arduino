#include "IOTPlatforma.h"

const char *ssid = "OpenWrt";
const char *password = "trollmode";

const char *API_KEY = "63fb1372230edb7275eeab599ba6ab89"; //test.iotplatforma.cloud - Testik

bool RELAY_STATE = false;
int REALY_PIN = 0;

void callbackSwitch(const JsonVariant &payload);
IOTPlatforma plat(API_KEY, IOTPlatforma::CONTROL_MODE);

const String keys[] = {"rgb"};
const String defaultData[] = {"{\"on\":1}"};

void setup() {
   pinMode(REALY_PIN, OUTPUT);
   setPin(LOW, REALY_PIN);

   plat.wifi(ssid, password);
    plat.setCallback("rgb", callbackRgb);
    // plat.clearTopic();
    // plat.saveTopic("/5d5554843beb3f419f9e4a64/garden/garage/gate");
    plat.enableOTA("123456777");
    plat.init();
    // TODO načíst předchozí uloženou barvu v EEPROM - asi přidat funkcionalitu do knihovny na uložení custom atributů
    plat.sendInitControl(defaultData, keys, 1);
}

void loop() {
     plat.loop();
}