#ifndef FIND_SLIVER_ELEMENTS_DIALOG_H
#define FIND_SLIVER_ELEMENTS_DIALOG_H

#include <QDialog>

#include "value_line_edit.h"

class FindSliverElementsDialog : public QDialog
{

        Q_OBJECT

        protected:

            //! Model ID.
            uint modelID;
            //! Tollerance edit.
            ValueLineEdit *edgeRatioEdit;

        public:

            //! Constructor.
            explicit FindSliverElementsDialog(uint modelID,QWidget *parent = nullptr);

            //! Overloaded exec function.
            int exec();

};

#endif // FIND_SLIVER_ELEMENTS_DIALOG_H
