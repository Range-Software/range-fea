#include "variable_selector.h"

VariableSelector::VariableSelector(const std::vector<RVariableType> &variableTypes, QWidget *parent) :
    QComboBox(parent)
{
    this->populate(variableTypes);

    this->connect(this,SIGNAL(currentIndexChanged(int)),SLOT(onCurrentIndexChanged(int)));
}

RVariableType VariableSelector::getCurrentVariableType() const
{
    return RVariableType(this->currentData().toInt());
}

bool VariableSelector::setCurrentVariableType(RVariableType variableType)
{
    for (int i=0;i<this->count();i++)
    {
        if (RVariableType(this->itemData(i).toInt()) == variableType)
        {
            this->setCurrentIndex(i);
            return true;
        }
    }
    return false;
}

void VariableSelector::populate(const std::vector<RVariableType> &variableTypes)
{
    this->blockSignals(true);
    this->clear();

    for (uint i=0;i<variableTypes.size();i++)
    {
        this->addItem(RVariable::getName(variableTypes[i]),QVariant(variableTypes[i]));
    }

    this->blockSignals(false);
}

void VariableSelector::onCurrentIndexChanged(int index)
{
    emit this->resultsVariableSelected(RVariableType(this->itemData(index).toInt()));
}
