#ifndef PUSH_BUTTON_H
#define PUSH_BUTTON_H

#include <QPushButton>

class PushButton : public QPushButton
{

    Q_OBJECT

    private:

        //! Push button ID.
        uint id;

    public:

        //! Constructor.
        explicit PushButton(uint id, const QString &text, QWidget *parent = nullptr);
        
    signals:
        
        //! Push button was clicked.
        void clicked(int id);

    private slots:

        //! Capture clicked signal.
        void onClicked();
        
};

#endif /* PUSH_BUTTON_H */
