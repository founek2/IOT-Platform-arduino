#include "Device.h"
#include "Node.h"
#include "NodeMy.h"

const char *Status::init = "init";
const char *Status::ready = "ready";
const char *Status::lost = "lost";
const char *Status::paired = "paired";
const char *Status::sleeping = "sleeping";
const char *Status::restarting = "restarting";
const char *Status::disconnected = "disconnected";

Device::Device(const char *name, MqttClient *cl) : Base("ESP-1111", name), client(cl)
{
#ifdef ESP8266
    uint64_t chipID = ESP.getChipId();
#elif ESP32
    uint64_t chipID = ESP.getEfuseMac();
#endif

    snprintf(this->id + 4, 9, "%llX", chipID);
    Serial.printf("DeviceID %s\n", this->id);
    this->_updateTopic();
};

Node *Device::NewNode(const char *nodeId, const char *name, NodeType type)
{
    NodeMy *node = new NodeMy(nodeId, name, type, this->client);
    this->_nodes.add(node);
    return node;
}

void Device::announce()
{
    Serial.println("Announcing device");
    this->client->publish((this->getTopic() + "/" + "$homie").c_str(), "4.0.0", true, 1);
    this->client->publish((this->getTopic() + "/" + "$name").c_str(), this->getName(), true, 1);
    this->client->publish((this->getTopic() + "/" + "$realm").c_str(), this->realm.c_str(), true, 1);

#ifdef ESP8266
    this->client->publish((this->getTopic() + "/" + "$implementation").c_str(), "esp8266", true, 1);
#elif ESP32
    this->client->publish((this->getTopic() + "/" + "$implementation").c_str(), "esp32", true, 1);
#endif

    String nodesList = "";
    for (int i = 0; i < this->_nodes.size(); i++)
    {
        auto node = this->_nodes.get(i);
        nodesList += node->getId();
        if (i < this->_nodes.size() - 1)
            nodesList += ",";
    }

    Serial.println(nodesList);
    if (this->_nodes.size() > 0)
        this->client->publish((this->getTopic() + "/" + "$nodes").c_str(), nodesList.c_str(), true, 1);

    for (int i = 0; i < this->_nodes.size(); i++)
    {
        auto node = this->_nodes.get(i);
        node->announce();
    }
}

void Device::setRealm(const char *realm)
{
    this->realm = realm;
}

const char *Device::getRealm()
{
    return this->realm.c_str();
}
void Device::setPrefix(const char *prefix)
{
    this->prefix = prefix;
    this->_updateTopic();
}

void Device::_updateTopic()
{
    this->setTopic(this->prefix + "/" + this->getId());

    for (int i = 0; i < this->_nodes.size(); i++)
    {
        Node *node = this->_nodes.get(i);
        node->setTopic(this->getTopic() + "/" + node->getId());
    }
}

void Device::publishStatus(const char *status)
{
    Serial.printf("publish status=%s\n", status);
    client->publish((this->getTopic() + "/" + "$state").c_str(), status, true, 1);
}

void Device::subscribe()
{
    for (int i = 0; i < this->_nodes.size(); i++)
    {
        auto node = this->_nodes.get(i);
        node->subscribe();
    }
}

void Device::handleSubscribe(const String &topic, byte *payload, unsigned int len)
{
    char data[len + 1];
    memcpy(data, payload, len);
    data[len] = '\0';
    for (int i = 0; i < this->_nodes.size(); i++)
    {
        auto node = this->_nodes.get(i);
        if (topic.startsWith(this->getTopic()))
            node->handleSubscribe(topic, data);
    }
}

const char *Device::getId()
{
    return this->id;
}