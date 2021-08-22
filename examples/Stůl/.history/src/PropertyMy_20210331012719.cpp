#include "PropertyMy.h"

PropertyMy::PropertyMy(const char *propertyId, const char *name, const char *datatype, PubSubClient *cl) : Property(propertyId, name, datatype), client(cl){};

void PropertyMy::announce()
{
    Serial.printf("announce property %s, topic %s\n", this->getId(), this->getTopic().c_str());

    const String topic = this->getTopic();
    client->publish((topic + "/" + "$name").c_str(), this->getName());
    client->publish((topic + "/" + "$class").c_str(), this->getClass());
    client->publish((topic + "/" + "$datatype").c_str(), this->getDatatype());

    if (this->getUnit()[0] != '\0')
        client->publish((topic + "/" + "$unit").c_str(), this->getUnit());
    if (!this->getFormat()[0] != '\0')
        client->publish((topic + "/" + "$format").c_str(), this->getFormat());
    if (this->isSettable() == true)
        client->publish((topic + "/" + "$settable").c_str(), "true");
}

void PropertyMy::subscribe()
{
    if (this->isSettable())
    {
        Serial.printf("Subscribe property id=%s\n", this->getId());
        this->client->subscribe((this->getTopic() + "/set").c_str());
    }
}

void PropertyMy::setValue(const char *value)
{
    Property::setValue(value);
    Serial.printf("Setting value %s\n", value);
    if (this->client->connected())
    {
        Serial.printf("publishing value %s\n", value);
        this->client->publish(this->getTopic().c_str(), value);
    }
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