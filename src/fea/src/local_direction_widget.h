#ifndef LOCAL_DIRECTION_WIDGET_H
#define LOCAL_DIRECTION_WIDGET_H

#include <QWidget>
#include <QGroupBox>

#include <rbl_local_direction.h>

#include "position_widget.h"
#include "direction_widget.h"

class LocalDirectionWidget : public QGroupBox
{

    Q_OBJECT

    protected:

        //! Position widget
        PositionWidget *positionWidget;
        //! Direction widget
        DirectionWidget *directionWidget;
        //! Cancel button.
        QPushButton *cancelButton;
        //! OK button.
        QPushButton *okButton;

    public:

        //! Constructor.
        explicit LocalDirectionWidget(const QString &title, const RLocalDirection &localDirection, QWidget *parent = nullptr);

        //! Return local direction.
        RLocalDirection getLocalDirection() const;

        //! Set local direction.
        void setLocalDirection(const RLocalDirection &localDirection);

    protected:

        //! Show event handler.
        void showEvent(QShowEvent *event);

    signals:

        //! Position changed.
        void changed(const RLocalDirection &localDirection);

        //! Close signal.
        void closed();

    private slots:

        //! On position changed.
        void onPositionChanged(const RR3Vector &);

        //! On direction changed.
        void onDirectionChanged(const RR3Vector &);

        //! On cancel button clicked.
        void onCancelButtonClicked();

        //! On ok button clicked.
        void onOkButtonClicked();

};

#endif // LOCAL_DIRECTION_WIDGET_H
