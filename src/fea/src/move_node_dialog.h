#ifndef MOVE_NODE_DIALOG_H
#define MOVE_NODE_DIALOG_H

#include <QDialog>
#include <QPushButton>

#include "position_widget.h"
#include "session_node_id.h"

class MoveNodeDialog : public QDialog
{

    Q_OBJECT

    protected:

        //! Mode ID.
        uint modelID;
        //! List of node IDs.
        const QList<uint> &nodeIDs;
        //! Possition widgets.
        QList<PositionWidget*> positionWidgets;
        //! OK button.
        QPushButton *okButton;

    public:

        //! Constructor.
        explicit MoveNodeDialog(uint modelID, const QList<uint> &nodeIDs, QWidget *parent = nullptr);

        //! Execute dialog.
        int exec();

    protected:

        //! Return map of node IDs to new positions.
        QMap<SessionNodeID,RR3Vector> findNodeIdPositionMap() const;

    private slots:

        //! Position changed.
        void onPositionChanged(const RR3Vector &);

};

#endif // MOVE_NODE_DIALOG_H
