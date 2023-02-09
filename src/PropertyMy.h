#include <MqttClient.h>
#include <Base.h>
#include <Property.h>

#ifndef PropertyMy_H
#define PropertyMy_H

class PropertyMy : public Property
{
    MqttClient *client;

public:
    PropertyMy(const char *propertyId, const char *name, DataType datatype, MqttClient *cl);

    bool setValue(const char *value);
    void announce();
    void subscribe();

    void handleSubscribe(const String &topic, const char *payload);
};

#endif