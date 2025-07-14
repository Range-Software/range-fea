#include "pick_value.h"

void PickValue::_init(const PickValue *pPickValue)
{
    if (pPickValue)
    {
        this->position = pPickValue->position;
        this->values = pPickValue->values;
    }
}

PickValue::PickValue()
{
    this->_init();
}

PickValue::PickValue(const RR3Vector &position, const RRVector &values)
    : position(position)
    , values(values)
{
    this->_init();
}

PickValue::PickValue(const PickValue &pickValue)
{
    this->_init(&pickValue);
}

PickValue::~PickValue()
{

}

PickValue &PickValue::operator =(const PickValue &pickValue)
{
    this->_init(&pickValue);
    return (*this);
}

const RR3Vector &PickValue::getPosition() const
{
    return this->position;
}

void PickValue::setPosition(const RR3Vector &position)
{
    this->position = position;
}

const RRVector &PickValue::getValues() const
{
    return this->values;
}

void PickValue::setValues(const RRVector &values)
{
    this->values = values;
}
