#ifndef STREAM_LINE_DIALOG_H
#define STREAM_LINE_DIALOG_H

#include <QDialog>
#include <QListWidget>

#include <rml_variable.h>

#include "position_widget.h"

class StreamLineDialog : public QDialog
{

    Q_OBJECT

    protected:

        //! Vector variables list widget.
        QListWidget *variableList;
        //! Stream line position.
        PositionWidget *positionWidget;
        //! OK button.
        QPushButton *okButton;
        //! Model ID.
        uint modelID;
        //! Entity ID.
        uint entityID;

    public:

        //! Constructor.
        explicit StreamLineDialog(uint modelID, QWidget *parent = nullptr);

        //! Constructor.
        explicit StreamLineDialog(uint modelID, uint entityID, QWidget *parent = nullptr);

        //! Execute dialog.
        int exec();

    private:

        //! Create dialog.
        void createDialog();

        //! Return selected variable type.
        RVariableType getVariableType() const;

    private slots:

        //! Position has changed.
        void onPositionChanged(const RR3Vector &position);

        //! Variable tree selection has changed.
        void onVariableListSelectionChanged();

};

#endif // STREAM_LINE_DIALOG_H
