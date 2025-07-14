#include "session_node_id.h"

void SessionNodeID::_init(const SessionNodeID *pSessionNodeID)
{
    if (pSessionNodeID)
    {
        this->mid = pSessionNodeID->mid;
        this->nid = pSessionNodeID->nid;
    }
}

SessionNodeID::SessionNodeID()
    : mid(0)
    , nid(0)
{
    this->_init();
}

SessionNodeID::SessionNodeID(uint mid, uint nid)
    : mid(mid)
    , nid(nid)
{
    this->_init();
}

SessionNodeID::SessionNodeID(const SessionNodeID &sessionNodeID)
{
    this->_init(&sessionNodeID);
}

SessionNodeID::~SessionNodeID()
{
}

SessionNodeID &SessionNodeID::operator =(const SessionNodeID &sessionNodeID)
{
    this->_init(&sessionNodeID);
    return (*this);
}

bool SessionNodeID::operator ==(const SessionNodeID &sessionNodeID) const
{
    return (this->mid == sessionNodeID.mid && this->nid == sessionNodeID.nid);
}

bool SessionNodeID::operator <(const SessionNodeID &sessionNodeID) const
{
    if (this->mid > sessionNodeID.mid)
    {
        return false;
    }
    else if (this->mid < sessionNodeID.mid)
    {
        return true;
    }
    if (this->nid < sessionNodeID.nid)
    {
        return true;
    }
    return false;
}

bool SessionNodeID::operator >(const SessionNodeID &sessionNodeID) const
{
    if (this->mid < sessionNodeID.mid)
    {
        return false;
    }
    else if (this->mid > sessionNodeID.mid)
    {
        return true;
    }
    if (this->nid > sessionNodeID.nid)
    {
        return true;
    }
    return false;
}

uint SessionNodeID::getMid() const
{
    return this->mid;
}

void SessionNodeID::setMid(uint mid)
{
    this->mid = mid;
}

uint SessionNodeID::getNid() const
{
    return this->nid;
}

void SessionNodeID::setNid(uint nid)
{
    this->nid = nid;
}
