#include <PubSubClient.h>
#include <Base.h>

#ifndef Property_H
#define Property_H

enum class PropertyClass
{
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
    ENUM
};

class Property : public Base
{
    DataType _datatype;
    String _unit;
    String _format;
    PropertyClass _class;
    String _value;
    bool _settable = false;
    std::function<void(Property *)> _callback = nullptr;

public:
    Property(const char *propertyId, const char *name, DataType datatype);
    void setUnit(const char *unit);
    void setFormat(const char *format);
    void setSettable(bool isSetable);
    void setClass(PropertyClass propertyClass);
    virtual bool setValue(const char *value);
    void setCallback(std::function<void(Property *)> callback);

    const char *getUnit();
    DataType getDatatype();
    const char *getFormat();
    PropertyClass getClass();
    bool isSettable();
    const String &getValue();
    std::function<void(Property *)> getCallback();
};

#endif