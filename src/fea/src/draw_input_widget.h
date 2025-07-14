#ifndef DRAW_INPUT_WIDGET_H
#define DRAW_INPUT_WIDGET_H

#include <QPushButton>
#include <QCheckBox>
#include <QStackedLayout>
#include <QTextEdit>

#include "draw_input_tree.h"
#include "local_direction_widget.h"
#include "position_widget.h"
#include "text_edit_widget.h"

class DrawInputWidget : public QWidget
{

    Q_OBJECT

    protected:

        //! Stacked layout.
        QStackedLayout *stackedLayout;
        //! Position widget.
        PositionWidget *positionWidget;
        //! Local direction widget.
        LocalDirectionWidget *localDirectionWidget;
        //! Position widget.
        TextEditWidget *textWidget;
        //! Layout widget.
        QWidget *layoutWidget;
        //! Draw input tree.
        DrawInputTree *tree;
        //! Merge near nodes check box.
        QCheckBox *mergeNodesCheck;
        //! Remove button.
        QPushButton *removeButton;
        //! OK button.
        QPushButton *okButton;

    public:

        //! Constructor.
        explicit DrawInputWidget(QWidget *parent = nullptr);

    protected slots:

        //! Catch position request signal.
        void onPositionRequest(const RR3Vector &position);

        //! Position widget has changed.
        void onPositionWidgetChanged(const RR3Vector &position);

        //! Position widget has closed.
        void onPositionWidgetClosed();

        //! Catch position request signal.
        void onLocalDirectionRequest(const RLocalDirection &localDirection);

        //! Local direction widget has changed.
        void onLocalDirectionWidgetChanged(const RLocalDirection &localDirection);

        //! Local direction direction widget has closed.
        void onLocalDirectionWidgetClosed();

        //! Catch text request signal.
        void onTextRequest(const QString &text);

        //! text widget has changed.
        void onTextWidgetChanged(const QString &text);

        //! text widget has closed.
        void onTextWidgetClosed();

        //! Catch tree selection changed signal.
        void onSelectionChanged(const QList<uint> &objectIDs);

        //! Catch input parameter changed.
        void onInputParameterChanged(uint objectID, uint);

        //! Remove button was clicked.
        void onRemoveClicked();

        //! OK button was clicked.
        void onOkClicked();

        //! Cancel button was clicked.
        void onCancelClicked();

};

#endif // DRAW_INPUT_WIDGET_H
