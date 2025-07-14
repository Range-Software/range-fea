#include <QGridLayout>
#include <QLabel>

#include "light_properties_widget.h"
#include "color_combo_box.h"
#include "direction_widget.h"
#include "position_widget.h"

LightPropertiesWidget::LightPropertiesWidget(const QString &title, const RGLLight &glLight, QWidget *parent) :
    QGroupBox(title,parent),
    glLight(glLight),
    id(0)
{
    this->setCheckable(true);
    this->setChecked(glLight.getEnabled());

    QGridLayout *lightLayout = new QGridLayout;
    this->setLayout(lightLayout);

    QLabel *ambientLabel = new QLabel(tr("Ambient") + ":");
    lightLayout->addWidget(ambientLabel,0,0,1,1);

    ColorComboBox *ambientColorComboBox = new ColorComboBox;
    ambientColorComboBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    ambientColorComboBox->setColor(glLight.getAmbient());
    lightLayout->addWidget(ambientColorComboBox,0,1,1,1);

    QLabel *difuseLabel = new QLabel(tr("Diffuse") + ":");
    lightLayout->addWidget(difuseLabel,1,0,1,1);

    ColorComboBox *diffuseColorComboBox = new ColorComboBox;
    diffuseColorComboBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    diffuseColorComboBox->setColor(glLight.getDiffuse());
    lightLayout->addWidget(diffuseColorComboBox,1,1,1,1);

    QLabel *specularLabel = new QLabel(tr("Specular") + ":");
    lightLayout->addWidget(specularLabel,2,0,1,1);

    ColorComboBox *specularColorComboBox = new ColorComboBox;
    specularColorComboBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    specularColorComboBox->setColor(glLight.getSpecular());
    lightLayout->addWidget(specularColorComboBox,2,1,1,1);

    PositionWidget *positionWidget = new PositionWidget("Position",glLight.getPosition());
    positionWidget->hideButtons();
    positionWidget->hidePickButton();
    positionWidget->setXRange(-10.0,10.0);
    positionWidget->setYRange(-10.0,10.0);
    positionWidget->setZRange(0.0,10.0);
    lightLayout->addWidget(positionWidget,3,0,1,2);

    DirectionWidget *directionWidget = new DirectionWidget("Direction",glLight.getDirection());
    directionWidget->hideButtons();
    lightLayout->addWidget(directionWidget,4,0,1,2);

    QObject::connect(this,&LightPropertiesWidget::toggled,this,&LightPropertiesWidget::lightToggled);
    this->connect(ambientColorComboBox,SIGNAL(colorChanged(QColor)),SLOT(ambientColorChanged(QColor)));
    this->connect(diffuseColorComboBox,SIGNAL(colorChanged(QColor)),SLOT(diffuseColorChanged(QColor)));
    this->connect(specularColorComboBox,SIGNAL(colorChanged(QColor)),SLOT(specularColorChanged(QColor)));
    QObject::connect(positionWidget,&PositionWidget::changed,this,&LightPropertiesWidget::positionChanged);
    QObject::connect(directionWidget,&DirectionWidget::changed,this,&LightPropertiesWidget::directionChanged);
}

const RGLLight &LightPropertiesWidget::getLight() const
{
    return this->glLight;
}

void LightPropertiesWidget::lightToggled(bool enabled)
{
    this->glLight.setEnabled(enabled);
    emit this->lightChanged(this->glLight);
    emit this->lightChanged(this->glLight,this->id);
}

void LightPropertiesWidget::ambientColorChanged(const QColor &color)
{
    this->glLight.setAmbient(color);
    emit this->lightChanged(this->glLight);
    emit this->lightChanged(this->glLight,this->id);
}

void LightPropertiesWidget::diffuseColorChanged(const QColor &color)
{
    this->glLight.setDiffuse(color);
    emit this->lightChanged(this->glLight);
    emit this->lightChanged(this->glLight,this->id);
}

void LightPropertiesWidget::specularColorChanged(const QColor &color)
{
    this->glLight.setSpecular(color);
    emit this->lightChanged(this->glLight);
    emit this->lightChanged(this->glLight,this->id);
}

void LightPropertiesWidget::positionChanged(const RR3Vector &position)
{
    this->glLight.setPosition(position);
    emit this->lightChanged(this->glLight);
    emit this->lightChanged(this->glLight,this->id);
}

void LightPropertiesWidget::directionChanged(const RR3Vector &direction)
{
    this->glLight.setDirection(direction);
    emit this->lightChanged(this->glLight);
    emit this->lightChanged(this->glLight,this->id);
}
