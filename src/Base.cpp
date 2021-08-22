#include "Base.h"

Base::Base(const char *_id, const char *_name) : id(_id), name(_name){};

const char *Base::getId()
{
    return this->id.c_str();
}
const char *Base::getName()
{
    return this->name.c_str();
}
const String &Base::getTopic()
{
    return this->topic;
}

void Base::setTopic(String topic)
{
    this->topic = topic;
}
