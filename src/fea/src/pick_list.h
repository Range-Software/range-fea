#ifndef PICK_LIST_H
#define PICK_LIST_H

#include <QObject>
#include <QVector>

#include "pick_item.h"

class PickList : public QObject
{
    Q_OBJECT

    protected:

        //! Multiple selection enabled.
        bool multipleSelection;
        //! List of picked items.
        QVector<PickItem> items;

    public:

        //! Constructor.
        explicit PickList(QObject *parent = nullptr);

        //! Return multiple selection boolean.
        bool getMultipleSelection() const;

        //! Set multiple selection boolean.
        void setMultipleSelection(bool multipleSelection);

        //! Return const reference to list of items.
        const QVector<PickItem> & getItems() const;

        //! Return list of items for specified model ID.
        QVector<PickItem> getItems(uint modelID) const;

        //! Return number of picked items of given type.
        uint getNItems(PickItemType type, uint modelID = RConstants::eod) const;

        //! Return true if pick list is empty.
        bool isEmpty() const;

        //! Return false if pick list contains items for specified model ID and for element group type if specified.
        bool isEmpty(uint modelID, REntityGroupType elementGroupType = R_ENTITY_GROUP_NONE) const;

        //! Return list of picked model IDs.
        QList<uint> getModelIDs() const;

        //! Register pick item to the list.
        //! If pick item is already present in the list it will be removed else added.
        //! If item is added true is returned.
        bool registerItem(const PickItem &pickItem);

        //! Add item to the list.
        void addItem(const PickItem &pickItem);

        //! Remove item from the list.
        void removeItem(const PickItem &pickItem);

        //! Remove items with given modelID from the list.
        void removeItems(uint modelID);

        //! Check if given pick item is in the list.
        bool hasItem(const PickItem &pickItem);

        //! Clear list.
        void clear();
        
    signals:

        //! Pick list has changed.
        void pickListChanged();
        
};

#endif // PICK_LIST_H
