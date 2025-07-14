#ifndef SESSION_NODE_ID_H
#define SESSION_NODE_ID_H

#include <QtGlobal>

class SessionNodeID
{

    protected:
        uint mid;
        uint nid;

    private:

        //! Internal initialization function.
        void _init(const SessionNodeID *pSessionNodeID = nullptr);

    public:

        //! Constructor.
        SessionNodeID();

        //! Constructor.
        SessionNodeID(uint mid, uint nid);

        //! Copy constructor.
        SessionNodeID(const SessionNodeID &sessionNodeID);

        //! Destructor.
        ~SessionNodeID();

        //! Assignment operator.
        SessionNodeID &operator =(const SessionNodeID &sessionNodeID);

        //! Compare operator.
        bool operator ==(const SessionNodeID &sessionNodeID) const;

        //! Compare less operator.
        bool operator <(const SessionNodeID &sessionNodeID) const;

        //! Compare greater operator.
        bool operator >(const SessionNodeID &sessionNodeID) const;

        //! Return model ID.
        uint getMid() const;

        //! Set model ID.
        void setMid(uint mid);

        //! Return node ID.
        uint getNid() const;

        //! Set node ID.
        void setNid(uint nid);

};

#endif // SESSION_NODE_ID_H
