#ifndef VALUE_SET_GENERATOR_DIALOG_H
#define VALUE_SET_GENERATOR_DIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QComboBox>

#include <rbl_value_table.h>

#include "value_line_edit.h"

class ValueSetGeneratorStaticValues;

class ValueSetGeneratorDialog : public QDialog
{

    Q_OBJECT

    protected:

        //! Static values.
        static QMap<QString,ValueSetGeneratorStaticValues> values;
        //! Value table.
        RValueTable &valueTable;
        //! Function combo box.
        QComboBox *funcComboBox;
        //! A line edit.
        ValueLineEdit *aLineEdit;
        //! B line edit.
        ValueLineEdit *bLineEdit;
        //! C line edit.
        ValueLineEdit *cLineEdit;
        //! D line edit.
        ValueLineEdit *dLineEdit;

    public:

        //! Constructor.
        explicit ValueSetGeneratorDialog(RValueTable &valueTable, QWidget *parent = nullptr);

        //! Execute dialog.
        int exec();

    protected slots:

        //! func combo index changed.
        void onFuncComboCurrentIndexChanged(int index);

};

#endif // VALUE_SET_GENERATOR_DIALOG_H
