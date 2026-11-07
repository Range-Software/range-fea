#ifndef PROBLEM_SELECTOR_DIALOG_H
#define PROBLEM_SELECTOR_DIALOG_H

#include <QDialog>

#include "problem_selector_tree.h"

class ProblemSelectorTiles;
class QToolButton;

class ProblemSelectorDialog : public QDialog
{

    Q_OBJECT

    protected:

        //! Problem selector tree widget.
        ProblemSelectorTree *problemTree;
        //! Problem selector tiles widget.
        ProblemSelectorTiles *problemTiles;
        //! Show problem types as a list button.
        QToolButton *listViewButton;
        //! Show problem types as tiles button.
        QToolButton *tileViewButton;
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

        //! Show problem types as tiles or as a list.
        void setTileView(bool tileView);

};

#endif /* PROBLEM_SELECTOR_DIALOG_H */
