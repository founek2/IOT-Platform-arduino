#include "Property.h"

Property::Property(const char *propertyId, const char *name, DataType datatype) : Base(propertyId, name), _datatype(datatype){};

void Property::setUnit(const char *unit)
{
    this->_unit = unit;
}
void Property::setFormat(const char *format)
{
    this->_format = format;
}
void Property::setSettable(bool isSetable)
{
    this->_settable = isSetable;
}
void Property::setClass(PropertyClass propertyClass)
{
    this->_class = propertyClass;
}

bool Property::setValue(const char *value)
{
    this->_value = value;

    return true;
}

void Property::setRetainable(const bool retainable)
{
    this->_retainable = retainable;
}

const String &Property::getValue()
{
    return this->_value;
}

void Property::setCallback(std::function<void(Property *)> callback)
{
    this->_callback = callback;
}

String &Property::getUnit()
{
    return this->_unit;
}
DataType Property::getDatatype()
{
    return this->_datatype;
}
String &Property::getFormat()
{
    return this->_format;
}
PropertyClass Property::getClass()
{
    return this->_class;
}
bool Property::isSettable()
{
    return this->_settable;
}

bool Property::isRetainable()
{
    return this->_retainable;
}

std::function<void(Property *)> Property::getCallback()
{
    return this->_callback;
}