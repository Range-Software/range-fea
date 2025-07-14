#include <QDateTime>

#include "solver_task_id.h"

void SolverTaskID::_init(const SolverTaskID *pSolverTaskID)
{
    if (pSolverTaskID)
    {
        this->year = pSolverTaskID->year;
        this->month = pSolverTaskID->month;
        this->day = pSolverTaskID->day;
        this->hour = pSolverTaskID->hour;
        this->minute = pSolverTaskID->minute;
        this->counter = pSolverTaskID->counter;
    }
}

SolverTaskID::SolverTaskID()
    : year(0)
    , month(0)
    , day(0)
    , hour(0)
    , minute(0)
    , counter(0)
{
    this->_init();
}

SolverTaskID::SolverTaskID(const SolverTaskID &solverTaskID)
{
    this->_init(&solverTaskID);
}

SolverTaskID::~SolverTaskID()
{
}

SolverTaskID &SolverTaskID::operator =(const SolverTaskID &solverTaskID)
{
    this->_init(&solverTaskID);
    return (*this);
}

bool SolverTaskID::operator ==(const SolverTaskID &solverTaskID) const
{
    return (this->year == solverTaskID.year &&
            this->month == solverTaskID.month &&
            this->day == solverTaskID.day &&
            this->hour == solverTaskID.hour &&
            this->minute == solverTaskID.minute &&
            this->counter == solverTaskID.counter);
}

void SolverTaskID::generate()
{
    static SolverTaskID lastID;

    QDateTime currDateTime(QDateTime::currentDateTime());

    QDate currDate(currDateTime.date());
    QTime currTime(currDateTime.time());

    this->year = currDate.year();
    this->month = currDate.month();
    this->day = currDate.day();
    this->hour = currTime.hour();
    this->minute = currTime.minute();
    this->counter = 1;

    if (lastID.isValid())
    {
        if (this->year == lastID.year &&
            this->month == lastID.month &&
            this->day == lastID.day &&
            this->hour == lastID.hour &&
            this->minute == lastID.minute)
        {
            this->counter = lastID.counter + 1;
        }
    }

    lastID = (*this);
}

bool SolverTaskID::isValid() const
{
    if (this->year    < 1 ||
        this->month   < 1 ||
        this->day     < 1 ||
        this->hour    < 0 ||
        this->minute  < 0 ||
        this->counter < 1)
    {
        return false;
    }
    else
    {
        return true;
    }
}

const QString SolverTaskID::toString() const
{
    QString stringID;

    stringID.append(QString("%1").arg(this->year,4,10,QChar('0')));
    stringID.append(QString("%1").arg(this->month,2,10,QChar('0')));
    stringID.append(QString("%1").arg(this->day,2,10,QChar('0')));
    stringID.append(QString("%1").arg(this->hour,2,10,QChar('0')));
    stringID.append(QString("%1").arg(this->minute,2,10,QChar('0')));
    stringID.append(QString("%1").arg(this->counter,4,10,QChar('0')));

    return stringID;
}
