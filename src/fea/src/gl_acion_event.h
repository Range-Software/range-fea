#ifndef GL_ACTION_EVENT_H
#define GL_ACTION_EVENT_H

#include <QObject>
#include <QKeyEvent>
#include <QMouseEvent>

typedef struct _GLActionCombination GLActionCombination;

class GLActionEvent : public QObject
{
    Q_OBJECT

    private:

        static const std::vector<GLActionCombination> glActionCombinations;

    protected:

        //! Key modifier.
        Qt::KeyboardModifiers keyModifiers;
        //! Key.
        int key;
        //! Mause buttons flag.
        Qt::MouseButtons buttons;
        //! Wheel is scrolling
        Qt::ScrollPhase scrollPhase;
        //! Even changed.
        bool actionChanged;

    public:

        enum Type
        {
            None = 0,
            Translate,
            TranslateZ,
            Rotate,
            Zoom,
            PickElement,
            PickNode,
            PickHoleElement,
            PickClear,
            nTypes
        };

    public:

        //! Constructor.
        GLActionEvent(QObject *parent = nullptr);

        //! Return action event type.
        GLActionEvent::Type getType() const;

        //! Check whether action event has changed.
        bool getChanged() const;

        //! Return key modifier.
        inline Qt::KeyboardModifiers getKeyModifiers() const
        {
            return this->keyModifiers;
        }

        //! Return key.
        inline int getKey() const
        {
            return this->key;
        }

        //! Return mouse buttons.
        inline Qt::MouseButtons getMouseButtons() const
        {
            return this->buttons;
        }

        //! Set Qt key event.
        void setKeyEvent(QKeyEvent *keyEvent, bool release);

        //! Set Qt mouse event.
        void setMouseEvent(QMouseEvent *mouseEvent, bool release);

        //! Set Qt wheel event.
        void setScrollPhase(Qt::ScrollPhase srcollPhase);

        //! Clear.
        void clear();

        //! Return current key mouse conbination
        QString getKeyMouseCombination() const;

        //! Return string representation of keay mouse combination.
        static QString findKeyMouseCombination(Qt::KeyboardModifiers keyModifiers,
                                               int key,
                                               Qt::MouseButtons buttons,
                                               Qt::ScrollPhase scrollPhase);

        //! Return string representations of keay mouse combination for specified action type.
        static QVector<QString> findKeyMouseCombinations(GLActionEvent::Type type);

        //! Get string representation of action type.
        static QString toString(GLActionEvent::Type type);

    signals:

        //! Emit signal that action has changed.
        void changed(GLActionEvent::Type type);

};

#endif /* GL_ACTION_EVENT_H */
