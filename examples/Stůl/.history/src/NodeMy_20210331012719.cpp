#include "NodeMy.h"

NodeMy::NodeMy(const char *nodeId, const char *name, const char *type, PubSubClient *cl) : Node(nodeId, name, type), client(cl){};

Property *NodeMy::NewProperty(const char *propertyId, const char *name, const char *datatype)
{
    PropertyMy *property = new PropertyMy(propertyId, name, datatype, this->client);
    this->_properties.add(property);
    return property;
}

void NodeMy::announce()
{
    Serial.printf("announce node %s, topic %s\n", this->getId(), this->getTopic().c_str());
    client->publish((this->getTopic() + "/" + "$name").c_str(), this->getName());
    client->publish((this->getTopic() + "/" + "$type").c_str(), this->getType());

    String propertiesList = "";
    for (int i = 0; i < this->_properties.size(); i++)
    {
        auto property = this->_properties.get(i);
        propertiesList += property->getId();
        if (i < this->_properties.size() - 1)
            propertiesList += ",";
    }
    Serial.printf("propertiesList %s\n", propertiesList.c_str());

    if (this->_properties.size() > 0)
        client->publish((this->getTopic() + "/" + "$properties").c_str(), propertiesList.c_str());

    for (int i = 0; i < this->_properties.size(); i++)
    {
        auto property = this->_properties.get(i);
        property->announce();
    }
}

void NodeMy::subscribe()
{
    for (int i = 0; i < this->_properties.size(); i++)
    {
        auto property = this->_properties.get(i);
        property->subscribe();
    }
}

void NodeMy::handleSubscribe(const String &topic, const char *payload)
{
    Serial.println("handle node");
    for (int i = 0; i < this->_properties.size(); i++)
    {
        auto property = this->_properties.get(i);
        if (topic.startsWith(this->getTopic()))
            property->handleSubscribe(topic, payload);
    }
}

void NodeMy::setTopic(String topic)
{
    Base::setTopic(topic);

    for (int j = 0; j < this->_properties.size(); j++)
    {
        auto property = this->_properties.get(j);
        property->setTopic(this->getTopic() + "/" + property->getId());
    }
}
