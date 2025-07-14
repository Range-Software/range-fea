#include <QVBoxLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QFileDialog>
#include <QSpinBox>
#include <QLabel>
#include <QTabWidget>
#include <QTreeWidget>
#include <QDialogButtonBox>

#include "application_settings_dialog.h"
#include "application_settings_widget.h"

ApplicationSettingsDialog::ApplicationSettingsDialog(ApplicationSettings *applicationSettings, QWidget *parent) :
    QDialog(parent)
{
    this->setWindowTitle(tr("Application settings"));
    this->resize(500,500);

    QIcon defaultIcon(":/icons/action/pixmaps/range-undo.svg");
    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout(mainLayout);

    ApplicationSettingsWidget *applicationSettingsWidget = new ApplicationSettingsWidget(applicationSettings);
    mainLayout->addWidget(applicationSettingsWidget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *defaultButton = new QPushButton(defaultIcon, tr("Default"));
    buttonBox->addButton(defaultButton,QDialogButtonBox::ResetRole);

    QPushButton *closeButton = new QPushButton(cancelIcon, tr("Close"));
    buttonBox->addButton(closeButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);

    QObject::connect(defaultButton,&QPushButton::clicked,applicationSettingsWidget,&ApplicationSettingsWidget::setDefaultValues);
}
