#ifndef DRAW_ENGINE_INPUT_H
#define DRAW_ENGINE_INPUT_H

#include <cfloat>

#include <rbl_local_direction.h>
#include <rbl_utils.h>

typedef struct _DrawEngineInputDataType
{
    bool b;
    int i;
    uint u;
    double d;
    double v[3];
    RLocalDirection localDirection;
    QString text;
} DrawEngineInputDataType;

class DrawEngineInput
{

    public:

        enum Type
        {
            Bool = 0,
            Int,
            Uint,
            Double,
            Vector,
            LocalDirection,
            Text
        };

    protected:

        //! Parameter data type.
        Type type;
        //! Parameter data.
        DrawEngineInputDataType value;
        DrawEngineInputDataType minValue;
        DrawEngineInputDataType maxValue;
        //! Parameter name.
        QString name;
        //! Parameter description.
        QString desc;
        //! Parameter units.
        QString units;

    private:

        //! Internal initialization function.
        void _init(const DrawEngineInput *pDrawEngineInput = nullptr);

    public:

        //! Constructor.
        DrawEngineInput(bool b, const QString &name = QString(), const QString &desc = QString(), const QString &units = QString());

        //! Constructor.
        DrawEngineInput(int i, const QString &name = QString(), const QString &desc = QString(), const QString &units = QString(), int minValue = INT_MIN, int maxValue = INT_MAX);

        //! Constructor.
        DrawEngineInput(uint u, const QString &name = QString(), const QString &desc = QString(), const QString &units = QString(), uint minValue = 0, uint maxValue = UINT_MAX);

        //! Constructor.
        DrawEngineInput(double d, const QString &name = QString(), const QString &desc = QString(), const QString &units = QString(), double minValue = -DBL_MAX, double maxValue = DBL_MAX);

        //! Constructor.
        DrawEngineInput(const RR3Vector &v, const QString &name = QString(), const QString &desc = QString(), const QString &units = QString());

        //! Constructor.
        DrawEngineInput(const RLocalDirection &localDirection, const QString &name = QString(), const QString &desc = QString(), const QString &units = QString());

        //! Constructor.
        DrawEngineInput(const QString &text, const QString &name = QString(), const QString &desc = QString(), const QString &units = QString());

        //! Copy constructor.
        DrawEngineInput(const DrawEngineInput &drawEngineInput);

        //! Assignment operator.
        DrawEngineInput & operator =(const DrawEngineInput &drawEngineInput);

        //! Assign bool value.
        //! Return true if value has the same type and is valid.
        bool setValue(bool b);

        //! Assign int value.
        //! Return true if value has the same type and is valid.
        bool setValue(int i);

        //! Assign uint value.
        //! Return true if value has the same type and is valid.
        bool setValue(uint u);

        //! Assign double value.
        //! Return true if value has the same type and is valid.
        bool setValue(double d);

        //! Assign vector value.
        //! Return true if value has the same type and is valid.
        bool setValue(const RR3Vector &v);

        //! Assign local direction value.
        //! Return true if value has the same type and is valid.
        bool setValue(const RLocalDirection &localDirection);

        //! Assign text value.
        //! Return true if value has the same type and valid.
        bool setValue(const QString &text);

        //! Return boolean.
        bool toBool(bool *isOk = nullptr) const;

        //! Return int.
        int toInt(bool *isOk = nullptr) const;

        //! Return uint.
        uint toUint(bool *isOk = nullptr) const;

        //! Return double.
        double toDouble(bool *isOk = nullptr) const;

        //! Return vector.
        RR3Vector toVector(bool *isOk = nullptr) const;

        //! Return local direction.
        RLocalDirection toLocalDirection(bool *isOk = nullptr) const;

        //! Return text.
        QString toText(bool *isOk = nullptr) const;

        //! Return input parameter type.
        Type getType() const;

        //! Return const reference to parameter name.
        const QString &getName() const;

        //! Return const reference to parameter description.
        const QString &getDescription() const;

        //! Return const reference to parameter units.
        const QString &getUnits() const;

    protected:

        //! Validate current value.
        bool isValid(bool value) const;

        //! Validate current value.
        bool isValid(int value) const;

        //! Validate current value.
        bool isValid(uint value) const;

        //! Validate current value.
        bool isValid(double value) const;

        //! Validate current value.
        bool isValid(const RR3Vector &value) const;

        //! Validate current value.
        bool isValid(const RLocalDirection &) const;

        //! Validate current value.
        bool isValid(const QString &value) const;

};

#endif // DRAW_ENGINE_INPUT_H
