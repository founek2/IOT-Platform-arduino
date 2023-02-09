#include <Arduino.h>

#ifndef Base_H
#define Base_H

class Base
{
    String id;
    String name;
    String topic;

public:
    Base(const char *_id, const char *_name);

    const char *getId();
    const char *getName();
    const String &getTopic();

    virtual void setTopic(String topic);
};

#endif