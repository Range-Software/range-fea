#ifndef DROP_RESULTS_DIALOG_H
#define DROP_RESULTS_DIALOG_H

#include <QDialog>
#include <QCheckBox>

class DropResultsDialog : public QDialog
{

    Q_OBJECT

    protected:

        QCheckBox *clearResultsCheckBox;
        QCheckBox *deleteResultsCheckBox;

    public:

        //! Constructor.
        explicit DropResultsDialog(QWidget *parent = nullptr);

        //! Execute dialog.
        int exec();
        
    signals:
        
    public slots:
        
};

#endif // DROP_RESULTS_DIALOG_H
