#include <cmath>
#include <cfloat>
#include <QLocale>

#include "value_line_edit.h"

ValueLineEdit::ValueLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    this->createTimer();
    this->setDoubleValidator();
    QObject::connect(this,&QLineEdit::textChanged,this,&ValueLineEdit::onTextChaged);
}

ValueLineEdit::ValueLineEdit(double min, double max, QWidget *parent)
    : QLineEdit(parent)
{
    this->createTimer();
    this->setValidator(new QDoubleValidator(min,max,1000,this));
    QObject::connect(this,&QLineEdit::textChanged,this,&ValueLineEdit::onTextChaged);
}

ValueLineEdit::ValueLineEdit(int min, int max, QWidget *parent)
    : QLineEdit(parent)
{
    this->createTimer();
    this->setValidator(new QIntValidator(min,max,this));
    QObject::connect(this,&QLineEdit::textChanged,this,&ValueLineEdit::onTextChaged);
}

void ValueLineEdit::setDoubleValidator()
{
    this->setValidator(new QDoubleValidator(-DBL_MAX,DBL_MAX,1000,this));
    this->paintBackground();
}

void ValueLineEdit::setIntValidator()
{
    this->setValidator(new QIntValidator(INT_MIN,INT_MAX,this));
    this->paintBackground();
}

void ValueLineEdit::setRange(double min, double max)
{
    this->setValidator(new QDoubleValidator(min,max,1000,this));
    this->paintBackground();
}

void ValueLineEdit::setRange(int min, int max)
{
    this->setValidator(new QIntValidator(min,max,this));
    this->paintBackground();
}

double ValueLineEdit::getMinimum() const
{
    return qobject_cast<const QDoubleValidator *>(this->validator())->bottom();
}

double ValueLineEdit::getMaximum() const
{
    return qobject_cast<const QDoubleValidator *>(this->validator())->top();
}

double ValueLineEdit::getValue() const
{
    return QLocale().toDouble(this->text());
}

void ValueLineEdit::setValue(double value)
{
    this->setText(QLocale().toString(value));
}

void ValueLineEdit::setValue(int value)
{
    this->setText(QLocale().toString(value));
}

void ValueLineEdit::setValue(uint value)
{
    this->setText(QLocale().toString(value));
}

QValidator::State ValueLineEdit::getValidatorState() const
{
    QString strContent = this->text();

    bool intermediateFound = false;

    for (int i=0;i<strContent.length();i++)
    {
        switch (this->validator()->validate(strContent,i))
        {
            case QValidator::Invalid:
                return QValidator::Invalid;
            case QValidator::Intermediate:
                intermediateFound = true;
                break;
            case QValidator::Acceptable:
            default:
                break;
        }
    }

    if (intermediateFound)
    {
        return QValidator::Intermediate;
    }

    return QValidator::Acceptable;
}

void ValueLineEdit::createTimer()
{
    this->timer = new QTimer(this);
    QObject::connect(this->timer,
                     &QTimer::timeout,
                     this,
                     &ValueLineEdit::onTimeout);
}

void ValueLineEdit::paintBackground()
{
    if (this->timer->isActive())
    {
        this->timer->stop();
    }
    switch (this->getValidatorState())
    {
        case QValidator::Invalid:
        {
            this->setStyleSheet("QLineEdit{background: red;}");
            this->timer->start(2000);
            break;
        }
        case QValidator::Intermediate:
        {
            this->setStyleSheet("QLineEdit{background: rgb(255, 200, 200);}");
            this->timer->start(2000);
            break;
        }
        case QValidator::Acceptable:
        default:
        {
            this->setStyleSheet("");
            break;
        }
    }
}

void ValueLineEdit::onTextChaged(QString text)
{
    this->paintBackground();
    emit this->valueChanged(QLocale().toDouble(text));
}

void ValueLineEdit::onTimeout()
{
    QString newText = this->text();
    newText.remove(newText.length()-1,1);
    this->setText(newText);
}
