#include <Base.h>

#ifndef Property_H
#define Property_H

enum class PropertyClass
{
    NONE,
    TEMPERATURE,
    HUMIDITY,
    PRESSURE,
    VOLTAGE
};
enum class DataType
{
    STRING,
    FLOAT,
    BOOLEAN,
    INTEGER,
    ENUM,
    COLOR
};

class Property : public Base
{
    String _value;
    String _unit;
    String _format;
    DataType _datatype;
    PropertyClass _class = PropertyClass::NONE;
    bool _settable = false;
    bool _retainable = false;

    std::function<bool(Property *)> _callback = nullptr;

public:
    Property(const char *propertyId, const char *name, DataType datatype);

    virtual bool setValue(const char *value);
    void setUnit(const char *unit);
    void setFormat(const char *format);
    void setClass(PropertyClass propertyClass);
    void setSettable(bool isSetable);
    void setRetainable(bool retain);

    void setCallback(std::function<bool(Property *)> callback);

    String &getUnit();
    DataType getDatatype();
    String &getFormat();
    PropertyClass getClass();
    bool isSettable();
    bool isRetainable();
    const String &getValue();
    std::function<bool(Property *)> getCallback();
};

#endif