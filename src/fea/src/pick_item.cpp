#include <rbl_logger.h>

#include "pick_item.h"

void PickItem::_init(const PickItem *pPickItem)
{
    R_LOG_TRACE;
    if (pPickItem)
    {
        this->entityID = pPickItem->entityID;
        this->elementID = pPickItem->elementID;
        this->elementPosition = pPickItem->elementPosition;
        this->nodeID = pPickItem->nodeID;
        this->nodePosition = pPickItem->nodePosition;
    }
}

PickItem::PickItem()
    : entityID(SessionEntityID())
    , elementID(RConstants::eod)
    , elementPosition(RConstants::eod)
    , nodeID(RConstants::eod)
    , nodePosition(RConstants::eod)
{
    R_LOG_TRACE;
    this->_init();
}

PickItem::PickItem(const SessionEntityID &entityID, uint elementID, uint elementPosition, uint nodeID, uint nodePosition)
    : entityID(entityID)
    , elementID(elementID)
    , elementPosition(elementPosition)
    , nodeID(nodeID)
    , nodePosition(nodePosition)
{
    R_LOG_TRACE;
    this->_init();
}

PickItem::PickItem(const PickItem &pickItem)
{
    R_LOG_TRACE;
    this->_init(&pickItem);
}

PickItem::~PickItem()
{
    R_LOG_TRACE;
}

PickItem &PickItem::operator =(const PickItem &pickItem)
{
    R_LOG_TRACE;
    this->_init(&pickItem);
    return (*this);
}

const SessionEntityID &PickItem::getEntityID() const
{
    R_LOG_TRACE;
    return this->entityID;
}

uint PickItem::getElementID() const
{
    R_LOG_TRACE;
    return this->elementID;
}

uint PickItem::getElementPosition() const
{
    R_LOG_TRACE;
    return this->elementPosition;
}

uint PickItem::getNodeID() const
{
    R_LOG_TRACE;
    return this->nodeID;
}

uint PickItem::getNodePosition() const
{
    R_LOG_TRACE;
    return this->nodePosition;
}

PickItemType PickItem::getItemType() const
{
    R_LOG_TRACE;
    if (this->entityID.getType() != R_ENTITY_GROUP_NONE)
    {
        if (this->elementPosition != RConstants::eod && this->elementID != RConstants::eod)
        {
            if (this->nodePosition != RConstants::eod && this->nodeID != RConstants::eod)
            {
                return PICK_ITEM_NODE;
            }
            return PICK_ITEM_ELEMENT;
        }
    }
    else
    {
        if (this->elementPosition != RConstants::eod && this->elementID != RConstants::eod)
        {
            return PICK_ITEM_HOLE_ELEMENT;
        }
    }
    return PICK_ITEM_NONE;
}

bool PickItem::operator ==(const PickItem &pickItem)
{
    R_LOG_TRACE;
    return (this->entityID == pickItem.entityID &&
            this->elementPosition == pickItem.elementPosition &&
            this->elementID == pickItem.elementID &&
            this->nodePosition == pickItem.nodePosition &&
            this->nodeID == pickItem.nodeID);
}
