#ifndef SOLVER_TASK_ID_H
#define SOLVER_TASK_ID_H

#include <QString>

class SolverTaskID
{

    protected:

        //! Year.
        int year;
        //! Month.
        int month;
        //! Day.
        int day;
        //! Hour.
        int hour;
        //! Minute.
        int minute;
        //! Counter.
        unsigned int counter;

    private:

        //! Internal initialization function.
        void _init ( const SolverTaskID *pSolverTaskID = nullptr );

    public:

        //! Constructor.
        SolverTaskID();

        //! Copy constructor.
        SolverTaskID(const SolverTaskID &solverTaskID);

        //! Destructor.
        ~SolverTaskID();

        //! Assignment operator.
        SolverTaskID & operator =(const SolverTaskID &solverTaskID);

        //! Compare equal operator.
        bool operator ==(const SolverTaskID &solverTaskID) const;

        //! Generate new ID.
        void generate();

        //! Return true if ID is valid.
        bool isValid() const;

        //! Convert to string.
        const QString toString() const;

};

#endif // SOLVER_TASK_ID_H
