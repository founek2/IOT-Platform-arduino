#include <Arduino.h>
template <typename T>
class my_template;

template <>
class my_template<int>
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
class my_template<float>
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
class my_template<String>
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
class my_template<bool>
{
    ...
};