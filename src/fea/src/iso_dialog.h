#ifndef ISO_DIALOG_H
#define ISO_DIALOG_H

#include <QDialog>

#include "model_tree_simple.h"
#include "value_line_edit.h"

class IsoDialog : public QDialog
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
        //! Variable value.
        ValueLineEdit *variableValue;
        //! OK button.
        QPushButton *okButton;
        //! Model ID.
        uint modelID;
        //! Entity ID.
        uint entityID;

    public:

        //! Constructor.
        explicit IsoDialog(uint modelID, QWidget *parent = nullptr);

        //! Constructor.
        explicit IsoDialog(uint modelID, uint entityID, QWidget *parent = nullptr);

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

#endif // ISO_DIALOG_H
