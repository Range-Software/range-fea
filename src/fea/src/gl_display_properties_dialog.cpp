#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QDialogButtonBox>

#include "gl_display_properties_dialog.h"
#include "gl_widget.h"
#include "color_combo_box.h"
#include "light_properties_widget.h"

typedef enum _PropertiesListColumn
{
    PROPERTIES_LIST_COLUMN_1,
    PROPERTIES_LIST_N_COLUMNS
} PropertiesListColumn;

GLDisplayPropertiesDialog::GLDisplayPropertiesDialog(GLWidget *pGlWidget, QWidget *parent)
    : QDialog(parent)
    , pGlWidget(pGlWidget)
{
    this->backupDisplayProperties = this->pGlWidget->getGLDisplayProperties();

    this->setWindowTitle(QString(tr("Display preferences")));
    this->resize(500,500);

    QIcon defaultIcon(":/icons/action/pixmaps/range-undo.svg");
    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout (mainLayout);

    this->propertyTree = new QTreeWidget;
    this->propertyTree->setHeaderHidden(true);
    this->propertyTree->setRootIsDecorated(false);
    this->propertyTree->setSelectionMode(QAbstractItemView::NoSelection);
    this->propertyTree->setColumnCount(PROPERTIES_LIST_N_COLUMNS);
    this->propertyTree->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    mainLayout->addWidget(this->propertyTree);
    this->populate();

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *defaultButton = new QPushButton(defaultIcon, tr("Default"));
    buttonBox->addButton(defaultButton,QDialogButtonBox::ResetRole);

    QPushButton *cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    buttonBox->addButton(cancelButton,QDialogButtonBox::RejectRole);

    QPushButton *okButton = new QPushButton(okIcon, tr("Ok"));
    buttonBox->addButton(okButton,QDialogButtonBox::AcceptRole);

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    mainLayout->addLayout(buttonsLayout);

    QObject::connect(buttonBox,&QDialogButtonBox::rejected,this,&QDialog::reject);
    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);

    QObject::connect(defaultButton,&QPushButton::clicked,this,&GLDisplayPropertiesDialog::onDefaultClicked);
}

int GLDisplayPropertiesDialog::exec()
{
    int retVal = QDialog::exec();

    if (retVal == QDialog::Accepted)
    {
        this->pGlWidget->getGLDisplayProperties().store();
    }
    else
    {
        this->pGlWidget->getGLDisplayProperties() = this->backupDisplayProperties;
    }
    this->pGlWidget->update();

    return retVal;
}

void GLDisplayPropertiesDialog::populate()
{
    // Items --------------------------------------------------------------
    QTreeWidgetItem *itemItems = new QTreeWidgetItem(this->propertyTree);
    QGroupBox *itemsGroupBox = new QGroupBox(tr("Items"));

    QVBoxLayout *itemsLayout = new QVBoxLayout;
    itemsGroupBox->setLayout(itemsLayout);

    // Show model edges
    QCheckBox *showModelEdgesCheckBox = new QCheckBox(tr("Show model edges"));
    showModelEdgesCheckBox->setChecked(this->pGlWidget->getGLDisplayProperties().getShowModelEdges());
    itemsLayout->addWidget(showModelEdgesCheckBox);
    QObject::connect(showModelEdgesCheckBox,&QCheckBox::clicked,this,&GLDisplayPropertiesDialog::onShowModelEdgesClicked);

    // Show model edges
    QCheckBox *showModelDimensionsCheckBox = new QCheckBox(tr("Show model dimensions"));
    showModelDimensionsCheckBox->setChecked(this->pGlWidget->getGLDisplayProperties().getShowModelDimensions());
    itemsLayout->addWidget(showModelDimensionsCheckBox);
    QObject::connect(showModelDimensionsCheckBox,&QCheckBox::clicked,this,&GLDisplayPropertiesDialog::onShowModelDimensionsClicked);

    // Show model edges
    QCheckBox *showModelGridCheckBox = new QCheckBox(tr("Show model grid"));
    showModelGridCheckBox->setChecked(this->pGlWidget->getGLDisplayProperties().getShowModelGrid());
    itemsLayout->addWidget(showModelGridCheckBox);
    QObject::connect(showModelGridCheckBox,&QCheckBox::clicked,this,&GLDisplayPropertiesDialog::onShowModelGridClicked);

    // Show model errors
    QCheckBox *showModelErrorsCheckBox = new QCheckBox(tr("Show model errors"));
    showModelErrorsCheckBox->setChecked(this->pGlWidget->getGLDisplayProperties().getShowErrors());
    itemsLayout->addWidget(showModelErrorsCheckBox);
    QObject::connect(showModelErrorsCheckBox,&QCheckBox::clicked,this,&GLDisplayPropertiesDialog::onShowModelErrorsClicked);

    // Show global axis
    QCheckBox *showGlobalAxisCheckBox = new QCheckBox(tr("Show global axis"));
    showGlobalAxisCheckBox->setChecked(this->pGlWidget->getGLDisplayProperties().getDrawGlobalAxis());
    itemsLayout->addWidget(showGlobalAxisCheckBox);
    QObject::connect(showGlobalAxisCheckBox,&QCheckBox::clicked,this,&GLDisplayPropertiesDialog::onShowGlobalAxisClicked);

    // Show local axis
    QCheckBox *showLocalAxisCheckBox = new QCheckBox(tr("Show local axis"));
    showLocalAxisCheckBox->setChecked(this->pGlWidget->getGLDisplayProperties().getDrawLocalAxis());
    itemsLayout->addWidget(showLocalAxisCheckBox);
    QObject::connect(showLocalAxisCheckBox,&QCheckBox::clicked,this,&GLDisplayPropertiesDialog::onShowLocalAxisClicked);

    this->propertyTree->setItemWidget(itemItems,PROPERTIES_LIST_COLUMN_1,itemsGroupBox);

    // Background color ---------------------------------------------------
    QTreeWidgetItem *itemBgColor = new QTreeWidgetItem(this->propertyTree);
    QGroupBox *bgGroupBox = new QGroupBox(tr("Background"));

    QGridLayout *bgLayout = new QGridLayout;
    bgGroupBox->setLayout(bgLayout);

    QLabel *bgColorLabel = new QLabel(tr("Color") + ":");
    bgLayout->addWidget(bgColorLabel,0,0,1,1);

    ColorComboBox *bgColorComboBox = new ColorComboBox;
    bgColorComboBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    bgColorComboBox->setColor(this->pGlWidget->getGLDisplayProperties().getBgColor());
    bgLayout->addWidget(bgColorComboBox,0,1,1,1);

    this->connect(bgColorComboBox,
                  SIGNAL(colorChanged(QColor)),
                  SLOT(onBgColorChanged(QColor)));

    // Background gradient
    QCheckBox *bgGradientCheckBox = new QCheckBox(tr("Gradient"));
    bgGradientCheckBox->setCheckState(this->pGlWidget->getGLDisplayProperties().getBgGradient() ? Qt::Checked : Qt::Unchecked);
    bgLayout->addWidget(bgGradientCheckBox,1,0,1,2);

    QObject::connect(bgGradientCheckBox,&QCheckBox::checkStateChanged,this,&GLDisplayPropertiesDialog::onBgGradientStateChanged);

    this->propertyTree->setItemWidget(itemBgColor,PROPERTIES_LIST_COLUMN_1,bgGroupBox);

    // Lights -------------------------------------------------------------
    for (uint i=0;i<this->pGlWidget->getGLDisplayProperties().getNLights();i++)
    {
        QTreeWidgetItem *itemLight = new QTreeWidgetItem(this->propertyTree);

        LightPropertiesWidget *lightGroupBox = new LightPropertiesWidget("Light: " + QLocale().toString(i+1),this->pGlWidget->getGLDisplayProperties().getLight(i));
        lightGroupBox->setID(i);

        this->propertyTree->setItemWidget(itemLight,PROPERTIES_LIST_COLUMN_1,lightGroupBox);

        this->connect(lightGroupBox,SIGNAL(lightChanged(RGLLight,uint)),SLOT(onLightChanged(RGLLight,uint)));
    }

    this->propertyTree->resizeColumnToContents(PROPERTIES_LIST_COLUMN_1);
}

void GLDisplayPropertiesDialog::onShowModelEdgesClicked(bool checked)
{
    this->pGlWidget->getGLDisplayProperties().setShowModelEdges(checked);
    this->pGlWidget->update();
}

void GLDisplayPropertiesDialog::onShowModelDimensionsClicked(bool checked)
{
    this->pGlWidget->getGLDisplayProperties().setShowModelDimensions(checked);
    this->pGlWidget->update();
}

void GLDisplayPropertiesDialog::onShowModelGridClicked(bool checked)
{
    this->pGlWidget->getGLDisplayProperties().setShowModelGrid(checked);
    this->pGlWidget->update();
}

void GLDisplayPropertiesDialog::onShowModelErrorsClicked(bool checked)
{
    this->pGlWidget->getGLDisplayProperties().setShowErrors(checked);
    this->pGlWidget->update();
}

void GLDisplayPropertiesDialog::onShowGlobalAxisClicked(bool checked)
{
    this->pGlWidget->getGLDisplayProperties().setDrawGlobalAxis(checked);
    this->pGlWidget->update();
}

void GLDisplayPropertiesDialog::onShowLocalAxisClicked(bool checked)
{
    this->pGlWidget->getGLDisplayProperties().setDrawLocalAxis(checked);
    this->pGlWidget->update();
}

void GLDisplayPropertiesDialog::onBgColorChanged(const QColor &color)
{
    this->pGlWidget->getGLDisplayProperties().setBgColor(color);
    this->pGlWidget->update();
}

void GLDisplayPropertiesDialog::onBgGradientStateChanged(Qt::CheckState state)
{
    this->pGlWidget->getGLDisplayProperties().setBgGradient(state == Qt::Checked);
    this->pGlWidget->update();
}

void GLDisplayPropertiesDialog::onLightChanged(const RGLLight &light, uint id)
{
    this->pGlWidget->getGLDisplayProperties().getLight(id) = light;
    this->pGlWidget->update();
}

void GLDisplayPropertiesDialog::onDefaultClicked()
{
    this->propertyTree->clear();
    QString fileName = this->pGlWidget->getGLDisplayProperties().getFileName();
    this->pGlWidget->getGLDisplayProperties().operator =(GLDisplayProperties());
    this->pGlWidget->getGLDisplayProperties().setFileName(fileName);
    this->populate();
    this->pGlWidget->update();
}
