#ifndef NEW_MODEL_DIALOG_H
#define NEW_MODEL_DIALOG_H

#include <QDialog>
#include <QLineEdit>

class NewModelDialog : public QDialog
{
    Q_OBJECT

    protected:

        //! Name editor.
        QLineEdit *editName;
        //! Description editor.
        QLineEdit *editDesc;

    public:

        //! Constructor.
        explicit NewModelDialog(QWidget *parent = nullptr);

        //! Execute dialog.
        int exec();

    private:

        //! Return model name.
        QString getName() const;

        //! Return model description.
        QString getDescription() const;
        
};

#endif /* NEW_MODEL_DIALOG_H */
