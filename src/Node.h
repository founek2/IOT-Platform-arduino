#include <Property.h>
#include <PropertyMy.h>
#include <LinkedList.h>
#include <Base.h>

#ifndef Node_H
#define Node_H

enum class NodeType
{
    ACTIVATOR,
    SWITCH,
    SENSOR,
    GENERIC
};

class Node : public Base
{
    NodeType _type;

public:
    Node(const char *nodeId, const char *name, NodeType type);

    virtual Property *NewProperty(const char *propertyId, const char *name, DataType atatype) = 0;

    virtual void setTopic(String topic) = 0;

    NodeType getType();
};

#endif