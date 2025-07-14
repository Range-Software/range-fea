#ifndef COLOR_COMBO_BOX_H
#define COLOR_COMBO_BOX_H

#include <QComboBox>
#include <QTreeWidgetItem>
#include <QColor>

class ColorComboBox : public QComboBox

{
    Q_OBJECT

    protected:

        //! Width to height factor.
        int whFactor;
        //! ID number.
        uint id;
        //! Active color.
        QColor color;

    public:

        //! Constructor.
        explicit ColorComboBox(QWidget *parent = nullptr);

        //! Return currently selected color.
        QColor getColor();

        //! Set color.
        void setColor(const QColor &color);

        //! Return name of the selected color.
        QString getColorName();

        //! Set color by name.
        void setColorName(const QString &colorName);

        //! Return ID.
        inline uint getID() const
        {
            return this->id;
        }

        //! Set ID.
        inline void setID(uint id)
        {
            this->id = id;
        }

    private:

        //! Create icon from color name.
        QIcon createIcon(const QString &colorName);

        //! Add new color.
        void addColor(const QColor &color);

        //! Add new color by its name.
        void addColor(const QString &colorName);

    signals:

        //! Color changed signal.
        void colorChanged(const QColor &color);

        //! Color changed signal.
        void colorChanged(const QColor &color, uint id);

    public slots:

        //! Current index has changed.
        void onCurrentIndexChanged(int index);

};

#endif /* COLOR_COMBO_BOX_H */
