#ifndef CUT_DIALOG_H
#define CUT_DIALOG_H

#include <QDialog>
#include <QDoubleSpinBox>

#include <rbl_plane.h>

#include "model_tree_simple.h"

class CutDialog : public QDialog
{

    Q_OBJECT

    protected:

        //! Model tree widget.
        ModelTreeSimple *modelTree;
        //! OK button.
        QPushButton *okButton;
        //! Cut plane.
        RPlane cutPlane;
        //! Model ID.
        uint modelID;
        //! Entity ID.
        uint entityID;

    public:

        //! Constructor.
        explicit CutDialog(uint modelID, uint entityID, QWidget *parent = nullptr);

    private slots:

        //! Accept clicked.
        void onAccept();

        //! Reject clicked.
        void onReject();

        //! Position changed.
        void onPositionChanged(const RR3Vector &position);

        //! Direction changed.
        void onDirectionChanged(const RR3Vector &direction);

        //! Model tree selection has changed.
        void onModelTreeSelectionChanged();

};

#endif // CUT_DIALOG_H
