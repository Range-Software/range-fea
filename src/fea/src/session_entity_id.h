#ifndef SESSION_ENTITY_ID_H
#define SESSION_ENTITY_ID_H

#include <QtGlobal>
#include <QList>

#include <rml_entity_group.h>

class SessionEntityID
{

    private:

        //! Internal initialization function.
        void _init ( const SessionEntityID *pSessionEntityID = nullptr );

    protected:
        uint              mid;
        REntityGroupType type;
        uint              eid;

    public:

        //! Constructor.
        SessionEntityID();

        //! Constructor.
        SessionEntityID(uint mid, REntityGroupType type, uint eid);

        //! Copy constructor.
        SessionEntityID(const SessionEntityID &sessionEntityID);

        //! Destructor.
        ~SessionEntityID();

        //! Assignment operator.
        SessionEntityID & operator =(const SessionEntityID &sessionEntityID);

        //! Compare operator.
        bool operator ==(const SessionEntityID &sessionEntityID) const;

        //! Compare less operator.
        bool operator <(const SessionEntityID &sessionEntityID) const;

        //! Compare greater operator.
        bool operator >(const SessionEntityID &sessionEntityID) const;

        //! Return model ID.
        uint getMid() const;

        //! Set model ID.
        void setMid(uint mid);

        //! Return model ID.
        REntityGroupType getType() const;

        //! Set model ID.
        void setType(REntityGroupType type);

        //! Return entity ID.
        uint getEid() const;

        //! Set entity ID.
        void setEid(uint eid);

        //! Filter entities from the list based on the type.
        static QList<SessionEntityID> filterList(const QList<SessionEntityID> &entityList,REntityGroupTypeMask entityGroupTypeMask = R_ENTITY_GROUP_ALL);

        //! Produce list of entity IDs.
        static QList<uint> getEntityIDs(const QList<SessionEntityID> &entityList,REntityGroupTypeMask entityGroupTypeMask = R_ENTITY_GROUP_ALL);

};

#endif /* SESSION_ENTITY_ID_H */
