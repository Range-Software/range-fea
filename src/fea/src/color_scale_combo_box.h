#ifndef COLOR_SCALE_COMBO_BOX_H
#define COLOR_SCALE_COMBO_BOX_H

#include <QComboBox>

class ColorScaleComboBox : public QComboBox
{

    Q_OBJECT

    protected:

        //! Width to height factor.
        int whFactor;

    public:

        //! Constructor.
        explicit ColorScaleComboBox(QWidget *parent = nullptr);

        //! Set color scale by name.
        void setColorScaleName(const QString &colorScaleName);

    protected:

        //! Create icon from color scale name.
        QIcon createIcon(const QString &colorScaleName);
        
};

#endif // COLOR_SCALE_COMBO_BOX_H
