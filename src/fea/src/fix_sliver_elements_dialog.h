#ifndef FIXSLIVERELEMENTSDIALOG_H
#define FIXSLIVERELEMENTSDIALOG_H

#include <QDialog>

#include "value_line_edit.h"

class FixSliverElementsDialog : public QDialog
{

        Q_OBJECT

        protected:

            //! Model ID.
            uint modelID;
            //! Tollerance edit.
            ValueLineEdit *edgeRatioEdit;

        public:

            //! Constructor.
            explicit FixSliverElementsDialog(uint modelID,QWidget *parent = nullptr);

            //! Overloaded exec function.
            int exec();

};

#endif // FIXSLIVERELEMENTSDIALOG_H
