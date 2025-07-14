#ifndef SCALAR_FIELD_DIALOG_H
#define SCALAR_FIELD_DIALOG_H

#include <QDialog>
#include <QSpinBox>

#include "model_tree_simple.h"

class ScalarFieldDialog : public QDialog
{

    Q_OBJECT

    protected:

        enum TreeColumn
        {
            Name = 0,
            Type,
            NColumns
        };

        //! Model tree widget.
        ModelTreeSimple *modelTree;
        //! Scalar variables tree widget.
        QTreeWidget *variableTree;
        //! Point size spin box.
        QSpinBox *pointSizeSpin;
        //! OK button.
        QPushButton *okButton;
        //! Model ID.
        uint modelID;
        //! Entity ID.
        uint entityID;

    public:

        //! Constructor.
        explicit ScalarFieldDialog(uint modelID, QWidget *parent = nullptr);

        //! Constructor.
        explicit ScalarFieldDialog(uint modelID, uint entityID, QWidget *parent = nullptr);

        //! Execute dialog.
        int exec();

    private:

        //! Create dialog.
        void createDialog();

        //! Return selected variable type.
        RVariableType getVariableType() const;

    protected slots:

        //! Variable tree selection has changed.
        void onVariableTreeSelectionChanged();

        //! Model tree selection has changed.
        void onModelTreeSelectionChanged();

};

#endif // SCALAR_FIELD_DIALOG_H
