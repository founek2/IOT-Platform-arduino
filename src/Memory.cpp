#include "Memory.h"
#include "EEPROM.h"
#include "Arduino.h"

Memory::Memory()
{
    EEPROM.begin(EEPROM_LEN);
};

void Memory::setPairStatus(PairStatus status)
{
    EEPROM.write(this->_addr_prefix, status);
    EEPROM.write(this->_addr_prefix + 1, status >> 8);

    EEPROM.commit();

    Serial.print("saving pairStatus: ");
    Serial.println(status);

    this->_loadPairStatus();
    Serial.println(this->_pair_status);
}

void Memory::setApiKey(const char *apiKey)
{
    for (size_t i = 0; i < apiKeyLen; i++)
    {
        EEPROM.put(this->_addr_api_key + i, apiKey[i]);
    }
    EEPROM.commit();

    Serial.print("saving apiKey: ");
    Serial.println(apiKey);

    this->_loadApiKey();
    Serial.println(this->_api_key);
    this->setPairStatus(PAIR_STATUS_PAIRED);
}

void Memory::setServerAndRealm(const char *server, const char *realm)
{
    Serial.print("saving server: ");
    Serial.println(server);

    this->setPairStatus(PAIR_STATUS_INIT);
    for (size_t i = 0; i < serverMaxLen; i++)
    {
        EEPROM.put(this->_addr_server + i, server[i]);
    }

    for (size_t i = 0; i < realmMaxLen; i++)
    {
        EEPROM.put(this->_addr_realm + i, realm[i]);
    }
    EEPROM.commit();

    Serial.print("saving topic: ");
    Serial.println(realm);

    this->_loadServerAndRealm();
    Serial.println(this->_mqtt_server);
    Serial.println(this->_realm);
}

void Memory::_loadPairStatus()
{
    uint16_t status;
    status = EEPROM.read(this->_addr_prefix);
    status |= (uint16)EEPROM.read(this->_addr_prefix + 1) << 8;
    if (status == PAIR_STATUS_INIT || status == PAIR_STATUS_PAIRED)
        this->_pair_status = (PairStatus)status;
    else
        this->_pair_status = PAIR_STATUS_BLANK;

    Serial.print("print status=");
    Serial.println(this->_pair_status);
}

void Memory::_loadServerAndRealm()
{
    for (size_t i = 0; i < serverMaxLen; i++)
    {
        this->_mqtt_server[i] = EEPROM.read(this->_addr_server + i);
    }

    for (size_t i = 0; i < realmMaxLen; i++)
    {
        this->_realm[i] = EEPROM.read(this->_addr_realm + i);
    }
}

void Memory::_loadApiKey()
{
    for (size_t i = 0; i < apiKeyLen; i++)
    {
        this->_api_key[i] = EEPROM.read(this->_addr_api_key + i);
    }
}

void Memory::loadEEPROM()
{
    this->_loadPairStatus();
    if (this->_pair_status == PAIR_STATUS_INIT || this->_pair_status == PAIR_STATUS_PAIRED)
    {
        Serial.println("prefix=conf_prefix_init");
        this->_loadServerAndRealm();

        Serial.print("server=");
        Serial.println(this->_mqtt_server);
        Serial.print("realm=");
        Serial.println(this->_realm);
    }

    if (this->_pair_status == PAIR_STATUS_PAIRED)
    {
        Serial.println("prefix=conf_prefix_paired");
        this->_loadApiKey();
        Serial.print("apiKey=");
        Serial.println(this->_api_key);
    }
}

PairStatus Memory::getPairStatus()
{
    return this->_pair_status;
}

const char *Memory::getApiKey()
{
    return this->_api_key;
}

const char *Memory::getServer()
{
    return this->_mqtt_server;
}

const char *Memory::getRealm()
{
    return this->_realm;
}

void Memory::clearEEPROM()
{
    for (unsigned int i = 0; i < EEPROM_LEN; i++)
    {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
}

bool Memory::isPaired()
{
    return this->_pair_status == PAIR_STATUS_PAIRED;
}

bool Memory::isInit()
{
    return this->_pair_status == PAIR_STATUS_INIT;
}