#include "PropertyMy.h"
#include "util.h"

PropertyMy::PropertyMy(const char *propertyId, const char *name, DataType datatype, MqttClient *cl) : Property(propertyId, name, datatype), client(cl){};

const char *dataTypeMap[] = {
    "string",
    "float",
    "boolean",
    "integer",
    "enum",
    "color"};

const char *propertyClassMap[] = {
    "",
    "temperature",
    "humidity",
    "pressure",
    "voltage"};

void PropertyMy::announce()
{
    Serial.printf("announce property %s, topic %s\n", this->getId(), this->getTopic().c_str());

    const String topic = this->getTopic();
    client->publish((topic + "/" + "$name").c_str(), this->getName(), true, 1);
    if (this->getClass() != PropertyClass::NONE)
        client->publish((topic + "/" + "$class").c_str(), propertyClassMap[to_underlying(this->getClass())], true, 1);
    client->publish((topic + "/" + "$datatype").c_str(), dataTypeMap[to_underlying(this->getDatatype())], true, 1);

    if (!this->getUnit().isEmpty())
        client->publish((topic + "/" + "$unit").c_str(), this->getUnit().c_str(), true, 1);
    if (!this->getFormat().isEmpty())
        client->publish((topic + "/" + "$format").c_str(), this->getFormat().c_str(), true, 1);
    if (this->isSettable() == true)
        client->publish((topic + "/" + "$settable").c_str(), "true", true, 1);
    if (this->isRetainable() == true)
        client->publish((topic + "/" + "$retained").c_str(), "true", true, 1);
    if (!this->getValue().isEmpty()) // TODO check if this works
        client->publish((topic + "/" + this->getName()).c_str(), this->getValue().c_str(), true, 1);
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