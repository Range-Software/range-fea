#ifndef VECTOR_FIELD_DIALOG_H
#define VECTOR_FIELD_DIALOG_H

#include <QDialog>
#include <QCheckBox>

#include "model_tree_simple.h"

class VectorFieldDialog : public QDialog
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
        //! Vector variables tree widget.
        QTreeWidget *variableTree;
        //! 3D type check box.
        QCheckBox *type3DCheckbox;
        //! OK button.
        QPushButton *okButton;
        //! Model ID.
        uint modelID;
        //! Entity ID.
        uint entityID;

    public:

        //! Constructor.
        explicit VectorFieldDialog(uint modelID, QWidget *parent = nullptr);

        //! Constructor.
        explicit VectorFieldDialog(uint modelID, uint entityID, QWidget *parent = nullptr);

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

#endif // VECTOR_FIELD_DIALOG_H
