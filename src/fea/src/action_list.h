#ifndef ACTION_LIST_H
#define ACTION_LIST_H

#include <rgl_action_list.h>

class ActionList : public RActionList
{
    Q_OBJECT

    public:

        //! Constructor.
        explicit ActionList(const QList<RAction::Definition> &definitions, QObject *parent = nullptr);

        void processAvailability();

};

#endif // ACTION_LIST_H
