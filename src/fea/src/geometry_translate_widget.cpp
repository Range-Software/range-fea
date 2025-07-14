#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>

#include "geometry_translate_widget.h"
#include "application.h"

const RR3Vector GeometryTranslateWidget::defaultTranslation = RR3Vector(0.0,0.0,0.0);

GeometryTranslateWidget::GeometryTranslateWidget(const RR3Vector &translation, QWidget *parent) :
    QWidget(parent)
{
    QVBoxLayout *vBoxLayout = new QVBoxLayout;
    this->setLayout(vBoxLayout);

    QGroupBox *groupBox = new QGroupBox(tr("Distance"));
    vBoxLayout->addWidget(groupBox);

    QGridLayout *layout = new QGridLayout;
    groupBox->setLayout(layout);

    QLabel *xDistanceLabel = new QLabel("X:");
    layout->addWidget(xDistanceLabel,0,0,1,1);

    this->xDistanceLineEdit = new ValueLineEdit;
    this->xDistanceLineEdit->setDoubleValidator();
    this->xDistanceLineEdit->setValue(translation[0]);
    layout->addWidget(this->xDistanceLineEdit,0,1,1,1);

    QLabel *xDistanceUnit = new QLabel("[m]");
    layout->addWidget(xDistanceUnit,0,2,1,1);

    QLabel *yDistanceLabel = new QLabel("Y:");
    layout->addWidget(yDistanceLabel,1,0,1,1);

    this->yDistanceLineEdit = new ValueLineEdit;
    this->yDistanceLineEdit->setDoubleValidator();
    this->yDistanceLineEdit->setValue(translation[1]);
    layout->addWidget(this->yDistanceLineEdit,1,1,1,1);

    QLabel *yDistanceUnit = new QLabel("[m]");
    layout->addWidget(yDistanceUnit,1,2,1,1);

    QLabel *zDistanceLabel = new QLabel("Z:");
    layout->addWidget(zDistanceLabel,2,0,1,1);

    this->zDistanceLineEdit = new ValueLineEdit;
    this->zDistanceLineEdit->setDoubleValidator();
    this->zDistanceLineEdit->setValue(translation[2]);
    layout->addWidget(this->zDistanceLineEdit,2,1,1,1);

    QLabel *zDistanceUnit = new QLabel("[m]");
    layout->addWidget(zDistanceUnit,2,2,1,1);

    QPushButton *originButton = new QPushButton(QIcon(":/icons/action/pixmaps/range-undo.svg"),tr("Origin"));
    layout->addWidget(originButton,3,0,1,3);

    QObject::connect(originButton,
                     &QPushButton::clicked,
                     this,
                     &GeometryTranslateWidget::setOrigin);

    QPushButton *centerButton = new QPushButton(QIcon(":/icons/file/pixmaps/range-undo.svg"),tr("Center"));
    layout->addWidget(centerButton,4,0,1,3);

    QObject::connect(centerButton,
                     &QPushButton::clicked,
                     this,
                     &GeometryTranslateWidget::setCenter);

    QPushButton *resetButton = new QPushButton(QIcon(":/icons/file/pixmaps/range-undo.svg"),tr("Reset"));
    layout->addWidget(resetButton,5,0,1,3);

    QObject::connect(resetButton,
                     &QPushButton::clicked,
                     this,
                     &GeometryTranslateWidget::resetDistances);

    this->resetDistances();

    QObject::connect(this->xDistanceLineEdit,
                     &ValueLineEdit::valueChanged,
                     this,
                     &GeometryTranslateWidget::onDistanceValueChanged);
    QObject::connect(this->yDistanceLineEdit,
                     &ValueLineEdit::valueChanged,
                     this,
                     &GeometryTranslateWidget::onDistanceValueChanged);
    QObject::connect(this->zDistanceLineEdit,
                     &ValueLineEdit::valueChanged,
                     this,
                     &GeometryTranslateWidget::onDistanceValueChanged);

    vBoxLayout->addStretch(10);
}

void GeometryTranslateWidget::setOrigin()
{
    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();
    double x = 0.0, y = 0.0, z = 0.0;
    for (int i=0;i<selectedModelIDs.size();i++)
    {
        double xmin, xmax, ymin, ymax, zmin, zmax;
        Application::instance()->getSession()->getModel(selectedModelIDs[i]).findNodeLimits(xmin,xmax,ymin,ymax,zmin,zmax);
        if (i == 0)
        {
            x = xmin;
            y = ymin;
            z = zmin;
        }
        x = std::min(x,xmin);
        y = std::min(y,ymin);
        z = std::min(z,zmin);
    }
    this->xDistanceLineEdit->setValue(-x);
    this->yDistanceLineEdit->setValue(-y);
    this->zDistanceLineEdit->setValue(-z);
}

void GeometryTranslateWidget::setCenter()
{
    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();
    double x = 0.0, y = 0.0, z = 0.0;
    for (int i=0;i<selectedModelIDs.size();i++)
    {
        double xc, yc, zc;
        Application::instance()->getSession()->getModel(selectedModelIDs[i]).findNodeCenter(xc,yc,zc);
        x += xc;
        y += yc;
        z += zc;
    }
    if (selectedModelIDs.size())
    {
        x /= double(selectedModelIDs.size());
        y /= double(selectedModelIDs.size());
        z /= double(selectedModelIDs.size());
    }
    this->xDistanceLineEdit->setValue(-x);
    this->yDistanceLineEdit->setValue(-y);
    this->zDistanceLineEdit->setValue(-z);
}

void GeometryTranslateWidget::resetDistances()
{
    this->xDistanceLineEdit->setValue(this->defaultTranslation[0]);
    this->yDistanceLineEdit->setValue(this->defaultTranslation[1]);
    this->zDistanceLineEdit->setValue(this->defaultTranslation[2]);
}

void GeometryTranslateWidget::onDistanceValueChanged(double)
{
    emit this->translationChanged(RR3Vector(this->xDistanceLineEdit->getValue(),
                                            this->yDistanceLineEdit->getValue(),
                                            this->zDistanceLineEdit->getValue()));
}
