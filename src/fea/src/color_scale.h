#ifndef COLOR_SCALE_H
#define COLOR_SCALE_H

#include <QColor>
#include <QObject>
#include <QList>

class ColorScale : public QObject
{

    Q_OBJECT

    protected:

        //! Scale name.
        QString scaleName;

    public:

        //! Constructor.
        explicit ColorScale(const QString &scaleName = QString(), QObject *parent = nullptr);

        //! Get color scale name.
        const QString & getName() const;

        //! Set color scale name.
        void setName(const QString &scaleName);

        //! Validate color scale.
        static bool validate(const QString &scaleName);

        //! Return list of possible color scales.
        static QList<QString> getColorScales();

        //! Return default color scale.
        static QString getDefaultColorScale();

        //! Return number of equal bands of a stepped color scale, 0 for a continuous one.
        static int getBandCount(const QString &scaleName);

        //! Return whether lines are drawn between bands of the color scale.
        static bool getBandOutline(const QString &scaleName);

        //! Return colors of bands of a stepped color scale from minimum to maximum.
        static QList<QColor> getBandColors(const QString &scaleName);

        //! Return color of the line between two bands.
        static QColor findBandLineColor(const QColor &lowerColor, const QColor &upperColor);
        
    signals:

        //! Scale has changed.
        void changed(const QString &scaleName);
        
    public slots:
        
};

#endif // COLOR_SCALE_H
