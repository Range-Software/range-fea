#include <cmath>

#include <QGridLayout>

#include "variable_value_edit.h"

VariableValueEdit::VariableValueEdit(RVariableType variableType, QWidget *parent) :
    QWidget(parent),
    type(variableType)
{
    QGridLayout *layout = new QGridLayout;
    this->setLayout(layout);

    RVariableDataType dataType = RVariable::getDataType(this->type);

    if (dataType == R_VARIABLE_DATA_INTEGER)
    {
        this->lineEdit = new ValueLineEdit();
        this->lineEdit->setIntValidator();
        layout->addWidget(this->lineEdit);

        QObject::connect(this->lineEdit,
                         &ValueLineEdit::valueChanged,
                         this,
                         &VariableValueEdit::onValueChaged);
    }
    else if (dataType == R_VARIABLE_DATA_UNSIGNED_INTEGER)
    {
        this->lineEdit = new ValueLineEdit();
        this->lineEdit->setRange(0,INT_MAX);
        layout->addWidget(this->lineEdit);

        QObject::connect(this->lineEdit,
                         &ValueLineEdit::valueChanged,
                         this,
                         &VariableValueEdit::onValueChaged);
    }
    else if (dataType == R_VARIABLE_DATA_DOUBLE)
    {
        this->lineEdit = new ValueLineEdit();
        this->lineEdit->setDoubleValidator();
        layout->addWidget(this->lineEdit);

        QObject::connect(this->lineEdit,
                         &ValueLineEdit::valueChanged,
                         this,
                         &VariableValueEdit::onValueChaged);
    }
    else if (dataType == R_VARIABLE_DATA_BOOLEAN)
    {
        this->checkBox = new QCheckBox;
        layout->addWidget(this->checkBox);

        QObject::connect(this->checkBox,
                         &QCheckBox::checkStateChanged,
                         this,
                         &VariableValueEdit::onStateChaged);
    }
}

void VariableValueEdit::setValue(double value)
{
    RVariableDataType dataType = RVariable::getDataType(this->type);

    if (dataType == R_VARIABLE_DATA_DOUBLE)
    {
        this->lineEdit->setValue(value);
    }
    else if (dataType == R_VARIABLE_DATA_INTEGER)
    {
        this->lineEdit->setValue(int(std::ceil(value)));
    }
    else if (dataType == R_VARIABLE_DATA_UNSIGNED_INTEGER)
    {
        this->lineEdit->setValue(uint(std::ceil(value)));
    }
    else if (dataType == R_VARIABLE_DATA_BOOLEAN)
    {
        this->checkBox->setChecked(value != 0.0);
    }
}

void VariableValueEdit::onValueChaged(double value)
{
    emit this->valueChanged(this->type,value);
}

void VariableValueEdit::onStateChaged(Qt::CheckState state)
{
    emit this->valueChanged(this->type,(state == Qt::Checked) ? 1.0 : 0.0);
}
