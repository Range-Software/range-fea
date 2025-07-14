#ifndef RENAME_MODEL_DIALOG_H
#define RENAME_MODEL_DIALOG_H

#include <QDialog>
#include <QLineEdit>

class RenameModelDialog : public QDialog
{
    Q_OBJECT

    protected:

		//! Model ID.
        uint modelId;
        //! Name editor.
        QLineEdit *editName;
        //! Description editor.
        QLineEdit *editDesc;

    public:

        //! Constructor.
        explicit RenameModelDialog(uint modelId, QWidget *parent = nullptr);

        //! Execute dialog.
        int exec();

    private:

        //! Return model name.
        QString getName() const;

        //! Return model description.
        QString getDescription() const;
        
};

#endif /* RENAME_MODEL_DIALOG_H */
