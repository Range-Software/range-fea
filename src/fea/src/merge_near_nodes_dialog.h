#ifndef MERGE_NEAR_NODES_DIALOG_H
#define MERGE_NEAR_NODES_DIALOG_H

#include <QDialog>

#include "value_line_edit.h"

class MergeNearNodesDialog : public QDialog
{

    Q_OBJECT

    protected:

        //! Model ID.
        uint modelID;
        //! Tollerance edit.
        ValueLineEdit *toleranceEdit;

    public:

        //! Constructor.
        explicit MergeNearNodesDialog(uint modelID,QWidget *parent = nullptr);

        //! Overloaded exec function.
        int exec();

};

#endif // MERGE_NEAR_NODES_DIALOG_H
