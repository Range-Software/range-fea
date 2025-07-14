#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>

#include <rbl_error.h>
#include <rbl_logger.h>

#include "convergence_graph_widget.h"
#include "graph_widget.h"

ConvergenceGraphWidget::ConvergenceGraphWidget(const QString &convergenceFileName, QWidget *parent) : QWidget(parent)
{
    QIcon clearIcon(":/icons/action/pixmaps/range-clear.svg");

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout (mainLayout);

    QLabel *label = new QLabel("Source: <i>" + convergenceFileName + "</i>");
    mainLayout->addWidget(label);

    this->dataColumnCombo = new QComboBox;
    this->dataColumnCombo->addItems(ConvergenceGraphObject::findDataColumns(convergenceFileName));
    this->dataColumnCombo->setCurrentIndex(0);
    mainLayout->addWidget(this->dataColumnCombo);

    this->connect(this->dataColumnCombo,SIGNAL(currentIndexChanged(int)),SLOT(onCurrentIndexChanged(int)));

    this->graphObject = new ConvergenceGraphObject;
    this->graphObject->setSourceFileName(convergenceFileName);
    this->graphObject->setSourceReadInterval(1000);

    GraphWidget *graphWidget = new GraphWidget(graphObject,false,this);
    mainLayout->addWidget(graphWidget);
    graphWidget->setPaintGraphLimits(false);
    graphWidget->setPaintY0(true);

    QPushButton *clearButton = new QPushButton(clearIcon, tr("Clear"));
    mainLayout->addWidget(clearButton);

    QObject::connect(clearButton,&QPushButton::clicked,this,&ConvergenceGraphWidget::onClearButtonClicked);
}

void ConvergenceGraphWidget::onCurrentIndexChanged(int index)
{
    this->graphObject->setDataColumn(index+1,true);
}

void ConvergenceGraphWidget::onClearButtonClicked()
{
    try
    {
        int response = QMessageBox::question(this,
                                          tr("Clear convergence log"),
                                          tr("Are you sure you want to clear convergence log?"),
                                          QMessageBox::Yes | QMessageBox::No,
                                          QMessageBox::No);
        if (response == QMessageBox::Yes)
        {
            this->graphObject->clearSourceFile();
        }
    }
    catch (RError &error)
    {
        RLogger::error("Failed to clear graph source. %s\n",error.getMessage().toUtf8().constData());
    }
}
