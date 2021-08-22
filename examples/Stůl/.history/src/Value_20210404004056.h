#include <Arduino.h>
template <typename T>
class MyValue;

template <>
class MyValue<int>
{
    int value;

public:
    int setValue(int val)
    {
        this->value = val;
    }

    int getValue()
    {
        return this->value;
    }
};

template <>
class MyValue<float>
{
    float value;

public:
    float setValue(float val)
    {
        this->value = val;
    }

    float getValue()
    {
        return this->value;
    }
};

template <>
class MyValue<String>
{
    String value;

public:
    void setValue(const String &val)
    {
        this->value = val;
    }

    String &getValue()
    {
        return this->value;
    }
};

template <>
class MyValue<bool>
{
    bool value;

public:
    void setValue(bool val)
    {
        this->value = val;
    }

    bool getValue()
    {
        return this->value;
    }
};