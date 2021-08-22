#include "Node.h"
#include "Property.h"
#include "PropertyMy.h"

Node::Node(const char *nodeId, const char *name, NodeType type) : Base(nodeId, name), _type(type){};

NodeType Node::getType()
{
    return this->_type;
}