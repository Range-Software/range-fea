#include "push_button.h"

PushButton::PushButton(uint id, const QString &text, QWidget *parent) :
    QPushButton(text, parent),
    id(id)
{
    QObject::connect(this,&QPushButton::clicked,this,&PushButton::onClicked);
}

void PushButton::onClicked()
{
    emit this->clicked(int(this->id));
}
