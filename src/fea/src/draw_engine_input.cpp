#include "draw_engine_input.h"

void DrawEngineInput::_init(const DrawEngineInput *pDrawEngineInput)
{
    if (pDrawEngineInput)
    {
        this->type = pDrawEngineInput->type;
        this->value = pDrawEngineInput->value;
        this->minValue = pDrawEngineInput->minValue;
        this->maxValue = pDrawEngineInput->maxValue;
        this->name = pDrawEngineInput->name;
        this->desc = pDrawEngineInput->desc;
        this->units = pDrawEngineInput->units;
    } 
}

DrawEngineInput::DrawEngineInput(bool b, const QString &name, const QString &desc, const QString &units)
    : type(DrawEngineInput::Bool)
    , name(name)
    , desc(desc)
    , units(units)
{
    this->value.b = b;
    this->_init();
}

DrawEngineInput::DrawEngineInput(int i, const QString &name, const QString &desc, const QString &units, int minValue, int maxValue)
    : type(DrawEngineInput::Int)
    , name(name)
    , desc(desc)
    , units(units)
{
    this->value.i = i;
    this->minValue.i = minValue;
    this->maxValue.i = maxValue;
    this->_init();
}

DrawEngineInput::DrawEngineInput(uint u, const QString &name, const QString &desc, const QString &units, uint minValue, uint maxValue)
    : type(DrawEngineInput::Uint)
    , name(name)
    , desc(desc)
    , units(units)
{
    this->value.u = u;
    this->minValue.u = minValue;
    this->maxValue.u = maxValue;
    this->_init();
}

DrawEngineInput::DrawEngineInput(double d, const QString &name, const QString &desc, const QString &units, double minValue, double maxValue)
    : type(DrawEngineInput::Double)
    , name(name)
    , desc(desc)
    , units(units)
{
    this->value.d = d;
    this->minValue.d = minValue;
    this->maxValue.d = maxValue;
    this->_init();
}

DrawEngineInput::DrawEngineInput(const RR3Vector &v, const QString &name, const QString &desc, const QString &units)
    : type(DrawEngineInput::Vector)
    , name(name)
    , desc(desc)
    , units(units)
{
    this->value.v[0] = v[0];
    this->value.v[1] = v[1];
    this->value.v[2] = v[2];
    this->_init();
}

DrawEngineInput::DrawEngineInput(const RLocalDirection &localDirection, const QString &name, const QString &desc, const QString &units)
    : type(DrawEngineInput::LocalDirection)
    , name(name)
    , desc(desc)
    , units(units)
{
    this->value.localDirection = localDirection;
}

DrawEngineInput::DrawEngineInput(const QString &text, const QString &name, const QString &desc, const QString &units)
    : type(DrawEngineInput::Text)
    , name(name)
    , desc(desc)
    , units(units)
{
    this->value.text = text;
    this->_init();
}

DrawEngineInput::DrawEngineInput(const DrawEngineInput &drawEngineInput)
{
    this->_init(&drawEngineInput);
}

DrawEngineInput & DrawEngineInput::operator =(const DrawEngineInput &drawEngineInput)
{
    this->_init(&drawEngineInput);
    return (*this);
}

bool DrawEngineInput::setValue(bool b)
{
    if (!this->isValid(b))
    {
        return false;
    }
    this->type = DrawEngineInput::Bool;
    this->value.b = b;
    return true;
}

bool DrawEngineInput::setValue(int i)
{
    if (!this->isValid(i))
    {
        return false;
    }
    this->type = DrawEngineInput::Int;
    this->value.i = i;
    return true;
}

bool DrawEngineInput::setValue(uint u)
{
    if (!this->isValid(u))
    {
        return false;
    }
    this->type = DrawEngineInput::Uint;
    this->value.u = u;
    return true;
}

bool DrawEngineInput::setValue(double d)
{
    if (!this->isValid(d))
    {
        return false;
    }
    this->type = DrawEngineInput::Double;
    this->value.d = d;
    return true;
}

bool DrawEngineInput::setValue(const RR3Vector &v)
{
    if (!this->isValid(v))
    {
        return false;
    }
    this->type = DrawEngineInput::Vector;
    this->value.v[0] = v[0];
    this->value.v[1] = v[1];
    this->value.v[2] = v[2];
    return true;
}

bool DrawEngineInput::setValue(const RLocalDirection &localDirection)
{
    if (!this->isValid(localDirection))
    {
        return false;
    }
    this->type = DrawEngineInput::LocalDirection;
    this->value.localDirection = localDirection;
    return true;
}

bool DrawEngineInput::setValue(const QString &text)
{
    if (!this->isValid(text))
    {
        return false;
    }
    this->type = DrawEngineInput::Text;
    this->value.text = text;
    return true;
}

bool DrawEngineInput::toBool(bool *isOk) const
{
    if (isOk)
    {
        (*isOk) = this->type == DrawEngineInput::Bool;
    }
    if (this->type == DrawEngineInput::Bool)
    {
        return this->value.b;
    }
    return bool();
}

int DrawEngineInput::toInt(bool *isOk) const
{
    if (isOk)
    {
        (*isOk) = this->type == DrawEngineInput::Int;
    }
    if (this->type == DrawEngineInput::Int)
    {
        return this->value.i;
    }
    return int();
}

uint DrawEngineInput::toUint(bool *isOk) const
{
    if (isOk)
    {
        (*isOk) = this->type == DrawEngineInput::Uint;
    }
    if (this->type == DrawEngineInput::Uint)
    {
        return this->value.u;
    }
    return uint();
}

double DrawEngineInput::toDouble(bool *isOk) const
{
    if (isOk)
    {
        (*isOk) = this->type == DrawEngineInput::Double;
    }
    if (this->type == DrawEngineInput::Double)
    {
        return this->value.d;
    }
    return double();
}

RR3Vector DrawEngineInput::toVector(bool *isOk) const
{
    if (isOk)
    {
        (*isOk) = this->type == DrawEngineInput::Vector;
    }
    if (this->type == DrawEngineInput::Vector)
    {
        return RR3Vector(this->value.v[0],this->value.v[1],this->value.v[2]);
    }
    return RR3Vector();
}

RLocalDirection DrawEngineInput::toLocalDirection(bool *isOk) const
{
    if (isOk)
    {
        (*isOk) = this->type == DrawEngineInput::LocalDirection;
    }
    if (this->type == DrawEngineInput::LocalDirection)
    {
        return this->value.localDirection;
    }
    return RLocalDirection(RR3Vector(0.0,0.0,0.0),RR3Vector(1.0,0.0,0.0));
}

QString DrawEngineInput::toText(bool *isOk) const
{
    if (isOk)
    {
        (*isOk) = this->type == DrawEngineInput::Text;
    }
    if (this->type == DrawEngineInput::Text)
    {
        return this->value.text;
    }
    return QString();
}

DrawEngineInput::Type DrawEngineInput::getType() const
{
    return this->type;
}

const QString &DrawEngineInput::getName() const
{
    return this->name;
}

const QString &DrawEngineInput::getDescription() const
{
    return this->desc;
}

const QString &DrawEngineInput::getUnits() const
{
    return this->units;
}

bool DrawEngineInput::isValid(bool value) const
{
    if (this->type != DrawEngineInput::Bool)
    {
        return false;
    }
    return (value == true || value == false);
}

bool DrawEngineInput::isValid(int value) const
{
    if (this->type != DrawEngineInput::Int)
    {
        return false;
    }
    return (value >= this->minValue.i && value <= this->maxValue.i);
}

bool DrawEngineInput::isValid(uint value) const
{
    if (this->type != DrawEngineInput::Uint)
    {
        return false;
    }
    return (value >= this->minValue.u && value <= this->maxValue.u);
}

bool DrawEngineInput::isValid(double value) const
{
    if (this->type != DrawEngineInput::Double)
    {
        return false;
    }
    return (value >= this->minValue.d && value <= this->maxValue.d);
}

bool DrawEngineInput::isValid(const RR3Vector &value) const
{
    if (this->type != DrawEngineInput::Vector)
    {
        return false;
    }
    return (value.size() == 3);
}

bool DrawEngineInput::isValid(const RLocalDirection &) const
{
    if (this->type != DrawEngineInput::LocalDirection)
    {
        return false;
    }
    return true;
}

bool DrawEngineInput::isValid(const QString &value) const
{
    if (this->type != DrawEngineInput::Text)
    {
        return false;
    }
    return !value.isEmpty();
}
