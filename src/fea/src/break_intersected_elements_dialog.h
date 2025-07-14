#ifndef BREAK_INTERSECTED_ELEMENTS_DIALOG_H
#define BREAK_INTERSECTED_ELEMENTS_DIALOG_H

#include <QDialog>
#include <QSpinBox>

class BreakIntersectedElementsDialog : public QDialog
{

    Q_OBJECT

    protected:

        //! Model ID.
        uint modelID;
        //! Number of iterations spinbox.
        QSpinBox *nIterationsSpin;

    public:

        //! Constructor.
        explicit BreakIntersectedElementsDialog(uint modelID, QWidget *parent = nullptr);

        //! Execute dialog.
        int exec();

};

#endif // BREAK_INTERSECTED_ELEMENTS_DIALOG_H
