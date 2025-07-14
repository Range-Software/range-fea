#include <QGuiApplication>

#include "gl_acion_event.h"

typedef struct _GLActionCombination
{
    //! Key modifier.
    Qt::KeyboardModifiers keyModifiers;
    //! Key.
    int key;
    //! Mouse buttons flag.
    Qt::MouseButtons buttons;
    //! Wheel scroll phase.
    Qt::ScrollPhase scrollPhase;
    //! OpenGL action event type.
    GLActionEvent::Type eventType;
} GLActionCombination;

const std::vector<GLActionCombination> GLActionEvent::glActionCombinations =
{
    { Qt::NoModifier,                          0, Qt::NoButton,     Qt::NoScrollPhase, GLActionEvent::None            },
    // Translate
    { Qt::NoModifier,                          0, Qt::LeftButton,   Qt::NoScrollPhase, GLActionEvent::Translate       },
    // Translate Z
    { Qt::ControlModifier,                     0, Qt::NoButton,     Qt::ScrollUpdate,  GLActionEvent::TranslateZ      },
    // Zoom
    { Qt::NoModifier,                          0, Qt::NoButton,     Qt::ScrollUpdate,  GLActionEvent::Zoom            },
    { Qt::ShiftModifier | Qt::AltModifier,     0, Qt::LeftButton,   Qt::NoScrollPhase, GLActionEvent::Zoom            },
    // Rotate
    { Qt::ShiftModifier,                       0, Qt::LeftButton,   Qt::NoScrollPhase, GLActionEvent::Rotate          },
    { Qt::NoModifier,                          0, Qt::MiddleButton, Qt::NoScrollPhase, GLActionEvent::Rotate          },
    // Pick element
    { Qt::ControlModifier,                     0, Qt::LeftButton,   Qt::NoScrollPhase, GLActionEvent::PickElement     },
    // Pick node
    { Qt::ControlModifier | Qt::AltModifier,   0, Qt::LeftButton,   Qt::NoScrollPhase, GLActionEvent::PickNode        },
    // Pick hole element
    { Qt::ControlModifier | Qt::ShiftModifier, 0, Qt::LeftButton,   Qt::NoScrollPhase, GLActionEvent::PickHoleElement },
    // Pick clear

    { Qt::ControlModifier | Qt::MetaModifier,  0, Qt::LeftButton,   Qt::NoScrollPhase, GLActionEvent::PickClear       },
    { Qt::ControlModifier,                     0, Qt::RightButton,  Qt::NoScrollPhase, GLActionEvent::PickClear       }
};

GLActionEvent::GLActionEvent(QObject *parent)
    : QObject(parent)
    , keyModifiers(Qt::NoModifier)
    , key(0)
    , buttons(Qt::NoButton)
    , scrollPhase(Qt::NoScrollPhase)
    , actionChanged(false)
{
}

GLActionEvent::Type GLActionEvent::getType() const
{
    for (auto &glActionCombination : glActionCombinations)
    {
        if (this->buttons      == glActionCombination.buttons &&
            this->keyModifiers == glActionCombination.keyModifiers &&
            this->scrollPhase  == glActionCombination.scrollPhase)
        {
            return glActionCombination.eventType;
        }
    }

    return GLActionEvent::None;
}

bool GLActionEvent::getChanged() const
{
    return actionChanged;
}

void GLActionEvent::setKeyEvent(QKeyEvent *keyEvent, bool release)
{
    GLActionEvent::Type prevType = this->getType();
    this->key = release ? 0 : keyEvent->key();
    this->keyModifiers = QGuiApplication::queryKeyboardModifiers();
    GLActionEvent::Type currType = this->getType();
    this->actionChanged = (prevType != currType);
    emit this->changed(currType);
}

void GLActionEvent::setMouseEvent(QMouseEvent *mouseEvent, bool release)
{
    GLActionEvent::Type prevType = this->getType();
    this->buttons = release ? Qt::NoButton : mouseEvent->buttons();
    GLActionEvent::Type currType = this->getType();
    this->actionChanged = (prevType != currType);
    emit this->changed(currType);
}

void GLActionEvent::setScrollPhase(Qt::ScrollPhase srcollPhase)
{
    GLActionEvent::Type prevType = this->getType();
    this->scrollPhase = srcollPhase;
    GLActionEvent::Type currType = this->getType();
    this->actionChanged = (prevType != currType);
    emit this->changed(currType);
}

void GLActionEvent::clear()
{
    GLActionEvent::Type prevType = this->getType();
    this->buttons = Qt::NoButton;
    this->keyModifiers = Qt::NoModifier;
    this->key = 0;
    this->scrollPhase = Qt::NoScrollPhase;
    GLActionEvent::Type currType = this->getType();
    this->actionChanged = (prevType != currType);
    emit this->changed(currType);
}

QString GLActionEvent::getKeyMouseCombination() const
{
    return GLActionEvent::findKeyMouseCombination(this->keyModifiers,this->key,this->buttons,this->scrollPhase);
}

QString GLActionEvent::findKeyMouseCombination(Qt::KeyboardModifiers keyModifiers, int key, Qt::MouseButtons buttons, Qt::ScrollPhase scrollPhase)
{
    int _key = key;
    if (_key == Qt::Key_Alt     ||
        _key == Qt::Key_AltGr   ||
        _key == Qt::Key_Control ||
        _key == Qt::Key_Shift   ||
        _key == Qt::Key_Meta)
    {
        _key = 0;
    }
    QString keyString(QKeySequence(int(keyModifiers) + _key).toString(QKeySequence::NativeText));
    QString mouseButtonString;

    if (buttons & Qt::LeftButton)
    {
        mouseButtonString += (mouseButtonString.length() > 0 ? "+" : "") + tr("Left mouse button");
    }
    if (buttons & Qt::MiddleButton)
    {
        mouseButtonString += (mouseButtonString.length() > 0 ? "+" : "") + tr("Middle mouse button");
    }
    if (buttons & Qt::RightButton)
    {
        mouseButtonString += (mouseButtonString.length() > 0 ? "+" : "") + tr("Right mouse button");
    }
    if (scrollPhase == Qt::ScrollUpdate)
    {
        mouseButtonString += (mouseButtonString.length() > 0 ? "+" : "") + tr("Mouse scroll");
    }

    return keyString + mouseButtonString;
}

QVector<QString> GLActionEvent::findKeyMouseCombinations(GLActionEvent::Type type)
{
    QVector<QString> combinations;
    for (auto &glActionCombination : glActionCombinations)
    {
        if (glActionCombination.eventType == type)
        {
            combinations.append(GLActionEvent::findKeyMouseCombination(glActionCombination.keyModifiers,glActionCombination.key,glActionCombination.buttons,glActionCombination.scrollPhase));
        }
    }
    return combinations;
}

QString GLActionEvent::toString(GLActionEvent::Type type)
{
    switch (type)
    {
        case GLActionEvent::None:
            return QString();
        case GLActionEvent::Translate:
            return tr("Translate");
        case GLActionEvent::TranslateZ:
            return tr("Translate in Z direction");
        case GLActionEvent::Rotate:
            return tr("Rotate");
        case GLActionEvent::Zoom:
            return tr("Zoom");
        case GLActionEvent::PickElement:
            return tr("Pick element");
        case GLActionEvent::PickNode:
            return tr("Pick node");
        case GLActionEvent::PickHoleElement:
            return tr("Pick hole element");
        case GLActionEvent::PickClear:
            return tr("Clear pick information");
        default:
            return QString();
    }
}
