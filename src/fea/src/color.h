#ifndef COLOR_H
#define COLOR_H

#include <QColor>

class Color : public QColor
{

    public:

        //! Constructor.
        Color();

        //! Constructor from rgba values.
        Color(int r, int g, int b, int a = 255);

        //! Constructor from QRgb class.
        Color(QRgb color);

        //! Constructor from color name.
        Color(const QString &name);

        //! Constructor from color name.
        Color(const char *name);

        //! Constructor from QColor class.
        Color(const QColor &color);

        //! Copy constructor.
        Color(const Color &color);

        //! Constructor from global color.
        Color(Qt::GlobalColor color);

        //! Copy operator.
        Color& operator=(const Color &color);

        //! Set random color.
        void randomize(bool fromGlobal = false, bool excludeWhiteAndBlack = true);

        //! Create random QColor object.
        static QColor random (bool fromGlobal = false, bool excludeWhiteAndBlack = true);

        //! Return list of global colors.
        static QList<Qt::GlobalColor> getGlobalColors(bool excludeWhiteAndBlack = false);

        //! Return list of paint colors (excluding white and black).
        static QList<Qt::GlobalColor> getPaintColors();
};

#endif // COLOR_H
