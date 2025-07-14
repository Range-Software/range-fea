#ifndef PROBLEM_SELECTOR_DIALOG_H
#define PROBLEM_SELECTOR_DIALOG_H

#include <QDialog>

#include "problem_selector_tree.h"

class ProblemSelectorDialog : public QDialog
{

    Q_OBJECT

    protected:

        //! Problem selector tree widget.
        ProblemSelectorTree *problemTree;
        //! OK button.
        QPushButton *okButton;
        //! Cancel button.
        QPushButton *cancelButton;

    public:

        //! Constructor.
        explicit ProblemSelectorDialog(QWidget *parent = nullptr);

        //! Execute dialog.
        RProblemTypeMask exec();

    protected slots:

        //! Handle ProblemSelectorTree::changed signal.
        void onProblemTreeChanged();

};

#endif /* PROBLEM_SELECTOR_DIALOG_H */
