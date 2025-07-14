#ifndef MARK_ENTITY_DIALOG_H
#define MARK_ENTITY_DIALOG_H

#include <QDialog>
#include <QRadioButton>
#include <QSpinBox>

#include <rml_entity_group.h>

#include "pick_item.h"

class MarkEntityDialog : public QDialog
{

    Q_OBJECT

    protected:

        //! Separation angle.
        static double separationAngle;
        //! Entity type.
        REntityGroupType entityType;
        //! Auto-mark radio button.
        QRadioButton *autoMarkRadio;
        //! Mark picked elements radio button.
        QRadioButton *pickedMarkRadio;
        //! Separation angle spin box.
        QSpinBox *separationAngleSpin;

    public:

        //! Constructor.
        explicit MarkEntityDialog(REntityGroupType entityType, QWidget *parent = nullptr);

        //! Execute dialog.
        int exec();

    private:

        //! Automark all entities.
        bool autoMarkEntities(uint modelID);

        //! Mark picked entities.
        bool markEntities(uint modelID, const QVector<PickItem> &pickList);

};

#endif // MARK_ENTITY_DIALOG_H
