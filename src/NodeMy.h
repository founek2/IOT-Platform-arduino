#include <Property.h>
#include <PropertyMy.h>
#include <LinkedList.h>
#include <PubSubClient.h>
#include <Node.h>

#ifndef NodeMy_H
#define NodeMy_H

class NodeMy : public Node
{
    PubSubClient *client;
    LinkedList<PropertyMy *> _properties;

public:
    NodeMy(const char *nodeId, const char *name, NodeType type, PubSubClient *);

    Property *NewProperty(const char *propertyId, const char *name, DataType datatype);

    void setTopic(String topic);

    void announce();
    void subscribe();

    void handleSubscribe(const String &topic, const char *payload);
};

#endif