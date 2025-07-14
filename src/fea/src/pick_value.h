#ifndef PICK_VALUE_H
#define PICK_VALUE_H

#include <rbl_r3vector.h>

class PickValue
{

    private:

        //! Pick position.
        RR3Vector position;
        //! Pick values.
        RRVector values;

    private:

        //! Internal initialization function.
        void _init(const PickValue *pPickValue = nullptr);

    public:

        //! Constructor.
        PickValue();

        //! Constructor.
        PickValue(const RR3Vector &position, const RRVector &values);

        //! Copy constructor.
        PickValue(const PickValue &pickValue);

        //! Destructor.
        ~PickValue();

        //! Assignment operator.
        PickValue &operator =(const PickValue &pickValue);

        //! Return const reference to pick position.
        const RR3Vector &getPosition() const;

        //! Set pick position.
        void setPosition(const RR3Vector &position);

        //! Return const reference to pick values.
        const RRVector &getValues() const;

        //! Set pick values.
        void setValues(const RRVector &values);

};

#endif // PICK_VALUE_H
