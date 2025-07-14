#include "results_variable_selector.h"
#include "application.h"

ResultsVariableSelector::ResultsVariableSelector(QWidget *parent) :
    VariableSelector(std::vector<RVariableType>(),parent)
{
    R_LOG_TRACE;
    this->populate();

    QObject::connect(Application::instance()->getSession(),&Session::modelAdded,this,&ResultsVariableSelector::onResultsChanged);
    QObject::connect(Application::instance()->getSession(),&Session::modelRemoved,this,&ResultsVariableSelector::onResultsChanged);
    QObject::connect(Application::instance()->getSession(),&Session::resultsChanged,this,&ResultsVariableSelector::onResultsChanged);
    QObject::connect(Application::instance()->getSession(),&Session::modelSelectionChanged,this,&ResultsVariableSelector::onResultsChanged);
}

void ResultsVariableSelector::populate(RVariableType selectedVariableType)
{
    R_LOG_TRACE;
    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    this->blockSignals(true);
    this->clear();

    uint itemIndex = 0;
    for (int i=0;i<selectedModelIDs.size();i++)
    {
        Model &model = Application::instance()->getSession()->getModel(selectedModelIDs[i]);
        uint nVariables = model.getNVariables();

        for (uint j=0;j<nVariables;j++)
        {
            RVariable &variable = model.getVariable(j);

            QString sizeType;

            if (variable.getNVectors() == 1)
            {
                sizeType = "Scalar";
            }
            else if (variable.getNVectors() > 1)
            {
                sizeType = "Vector [" + QLocale().toString(variable.getNVectors()) + "]";
            }
            else
            {
                sizeType = "Unknown";
            }

            this->addItem(variable.getName() + " - " + sizeType,QVariant(variable.getType()));

            if (selectedVariableType == variable.getType())
            {
                this->setCurrentIndex(itemIndex);
            }
            itemIndex++;
        }
    }

    this->blockSignals(false);

    // Force signal even if current index has not changed.
    emit this->currentIndexChanged(this->currentIndex());
}

void ResultsVariableSelector::onResultsChanged(uint)
{
    R_LOG_TRACE;
    RVariableType selectedVariableType = RVariableType(this->itemData(this->currentIndex()).toInt());
    this->clear();
    this->populate(selectedVariableType);
}
