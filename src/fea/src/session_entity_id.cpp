#include "session_entity_id.h"

SessionEntityID::SessionEntityID()
    : mid(0)
    , type(R_ENTITY_GROUP_NONE)
    , eid(0)
{
    this->_init();
}

SessionEntityID::SessionEntityID(uint mid, REntityGroupType type, uint eid)
    : mid(mid)
    , type(type)
    , eid(eid)
{
}

SessionEntityID::SessionEntityID(const SessionEntityID &sessonEntityID)
{
    this->_init(&sessonEntityID);
}

SessionEntityID::~SessionEntityID()
{
}

void SessionEntityID::_init(const SessionEntityID *pSessionEntityID)
{
    if (pSessionEntityID)
    {
        this->setType(pSessionEntityID->getType());
        this->setMid(pSessionEntityID->getMid());
        this->setEid(pSessionEntityID->getEid());
    }
}

SessionEntityID & SessionEntityID::operator =(const SessionEntityID &sessonEntityID)
{
    this->_init(&sessonEntityID);
    return (*this);
}

bool SessionEntityID::operator ==(const SessionEntityID &sessionEntityID) const
{
    if (this->getType() == sessionEntityID.getType() &&
        this->getMid() == sessionEntityID.getMid() &&
        this->getEid() == sessionEntityID.getEid())
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool SessionEntityID::operator <(const SessionEntityID &sessionEntityID) const
{
    if (this->getMid() > sessionEntityID.getMid())
    {
        return false;
    }
    else if (this->getMid() < sessionEntityID.getMid())
    {
        return true;
    }
    else
    {
        if (this->getType() > sessionEntityID.getType())
        {
            return false;
        }
        else if (this->getType() < sessionEntityID.getType())
        {
            return true;
        }
        else
        {
            if (this->getEid() >= sessionEntityID.getEid())
            {
                return false;
            }
            else
            {
                return true;
            }
        }
    }
}

bool SessionEntityID::operator >(const SessionEntityID &sessionEntityID) const
{
    if (this->getMid() < sessionEntityID.getMid())
    {
        return false;
    }
    else if (this->getMid() > sessionEntityID.getMid())
    {
        return true;
    }
    else
    {
        if (this->getType() < sessionEntityID.getType())
        {
            return false;
        }
        else if (this->getType() > sessionEntityID.getType())
        {
            return true;
        }
        else
        {
            if (this->getEid() <= sessionEntityID.getEid())
            {
                return false;
            }
            else
            {
                return true;
            }
        }
    }
}

uint SessionEntityID::getMid() const
{
    return this->mid;
}

void SessionEntityID::setMid(uint mid)
{
    this->mid = mid;
}

REntityGroupType SessionEntityID::getType() const
{
    return this->type;
}

void SessionEntityID::setType(REntityGroupType type)
{
    this->type = type;
}

uint SessionEntityID::getEid() const
{
    return this->eid;
}

void SessionEntityID::setEid(uint eid)
{
    this->eid = eid;
}

QList<SessionEntityID> SessionEntityID::filterList(const QList<SessionEntityID> &entityList, REntityGroupTypeMask entityGroupTypeMask)
{
    QList<SessionEntityID> filteredList;

    for (int i=0;i<entityList.size();i++)
    {
        if (entityList[i].getType() & entityGroupTypeMask)
        {
            filteredList.push_back(entityList[i]);
        }
    }

    return filteredList;
}

QList<uint> SessionEntityID::getEntityIDs(const QList<SessionEntityID> &entityList, REntityGroupTypeMask entityGroupTypeMask)
{
    QList<uint> entityIDs;

    for (int i=0;i<entityList.size();i++)
    {
        if (entityList[i].getType() & entityGroupTypeMask)
        {
            entityIDs.push_back(entityList[i].getEid());
        }
    }

    return entityIDs;
}
