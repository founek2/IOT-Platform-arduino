#include "PropertyMy.h"
#include "util.h"

PropertyMy::PropertyMy(const char *propertyId, const char *name, DataType datatype, PubSubClient *cl) : Property(propertyId, name, datatype), client(cl){};

const char *dataTypeMap[] = {
    "string",
    "float",
    "boolean",
    "integer",
    "enum"};

const char *propertyClassMap[] = {
    "temperature",
    "humidity",
    "pressure",
    "voltage"};

void PropertyMy::announce()
{
    Serial.printf("announce property %s, topic %s\n", this->getId(), this->getTopic().c_str());

    const String topic = this->getTopic();
    client->publish((topic + "/" + "$name").c_str(), this->getName(), true);
    client->publish((topic + "/" + "$class").c_str(), propertyClassMap[to_underlying(this->getClass())], true);
    client->publish((topic + "/" + "$datatype").c_str(), dataTypeMap[to_underlying(this->getDatatype())], true);

    if (this->getUnit()[0] != '\0')
        client->publish((topic + "/" + "$unit").c_str(), this->getUnit(), true);
    if (this->getFormat()[0] != '\0')
        client->publish((topic + "/" + "$format").c_str(), this->getFormat(), true);
    if (this->isSettable() == true)
        client->publish((topic + "/" + "$settable").c_str(), "true", true);
}

void PropertyMy::subscribe()
{
    if (this->isSettable())
    {
        Serial.printf("Subscribe property id=%s\n", this->getId());
        this->client->subscribe((this->getTopic() + "/set").c_str());
    }
}

bool PropertyMy::setValue(const char *value)
{
    Property::setValue(value);
    Serial.printf("Setting value %s\n", value);
    if (this->client->connected())
    {
        Serial.printf("publishing value %s\n", value);
        return this->client->publish(this->getTopic().c_str(), value);
    }

    return false;
}

void PropertyMy::handleSubscribe(const String &topic, const char *payload)
{
    this->setValue(payload);
    if (this->getCallback() != nullptr)
    {
        this->getCallback()(this);
    }
    else
        Serial.printf("callback not defined for propertyId=%s", this->getId());
}