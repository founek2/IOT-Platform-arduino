#include <Node.h>
#include <NodeMy.h>
#include <LinkedList.h>
#include <MqttClient.h>

#include <Base.h>

#ifndef Device_H
#define Device_H

class Status
{
public:
    static const char *init;
    static const char *ready;
    static const char *lost;
    static const char *paired;
    static const char *sleeping;
    static const char *restarting;
    static const char *disconnected;
};

class Device : public Base
{
    char id[4 + 13] = {'E', 'S', 'P', '-', '\0'};
    String deviceId;
    String realm;

    String prefix;
    String api_key;

    LinkedList<NodeMy *> _nodes;
    MqttClient *client;

    void _updateTopic();

public:
    Device(const char *name, MqttClient *client);
    Node *NewNode(const char *nodeId, const char *name, NodeType type);

    void publishStatus(const char *status);

    const char *getId();
    void announce();
    void subscribe();

    void setRealm(const char *realm);
    const char *getRealm();
    void setPrefix(const char *prefix);

    void handleSubscribe(const String &topic, byte *payload, unsigned int len);
};

#endif