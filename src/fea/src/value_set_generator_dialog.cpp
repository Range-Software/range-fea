#include <cmath>

#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QDialogButtonBox>

#include <rbl_utils.h>

#include "value_set_generator_dialog.h"

typedef enum _FuncType
{
    FUNC_FILL_A = 0,
    FUNC_LINEAR,
    FUNC_SIN,
    FUNC_COS,
    FUNC_N_FUNCS
} FuncType;

class ValueSetGeneratorStaticValues
{
    public:
        FuncType funcType;
        double a;
        double b;
        double c;
        double d;
    public:
        ValueSetGeneratorStaticValues()
        {
            this->initialize();
        }
        void initialize()
        {
            this->funcType = FUNC_SIN;
            this->a = 0.0;
            this->b = 1.0;
            this->c = 0.0;
            this->d = 1.0;
        }
};

QMap<QString,ValueSetGeneratorStaticValues> ValueSetGeneratorDialog::values = QMap<QString,ValueSetGeneratorStaticValues>();

static double funcFindValue(FuncType funcType, const ValueSetGeneratorStaticValues &values, double key)
{
    switch (funcType)
    {
        case FUNC_FILL_A:
        {
            return values.a;
        }
        case FUNC_LINEAR:
        {
            return values.a+values.b*key;
        }
        case FUNC_SIN:
        {
            return values.a+values.b+sin(RConstants::pi*(values.c+values.d*key));
        }
        case FUNC_COS:
        {
            return values.a+values.b+cos(RConstants::pi*(values.c+values.d*key));
        }
        default:
        {
            return 0.0;
        }
    }
}

static QString getFuncName(FuncType funcType, const QString &keySymbol)
{
    switch (funcType)
    {
        case FUNC_FILL_A:
        {
            return "A";
        }
        case FUNC_LINEAR:
        {
            return "A+B*" + keySymbol;
        }
        case FUNC_SIN:
        {
            return "A+B*sin(" + QString(QChar(0xC0, 0x03)) + "*(C+D*" + keySymbol + "))";
        }
        case FUNC_COS:
        {
            return "A+B*cos(" + QString(QChar(0xC0, 0x03)) + "*(C+D*" + keySymbol + "))";
        }
        default:
        {
            return QString();
        }
    }
}

ValueSetGeneratorDialog::ValueSetGeneratorDialog(RValueTable &valueTable, QWidget *parent)
    : QDialog(parent)
    , valueTable(valueTable)
{
    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    QString windowTitleStr = tr("Value set generator");
    this->setWindowTitle(tr(windowTitleStr.toUtf8().constData()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout(mainLayout);

    QFormLayout *formLayout = new QFormLayout;
    mainLayout->addLayout(formLayout);

    QString key(this->valueTable.getKeyName() + "/" + this->valueTable.getValueName());
    bool isNew = !ValueSetGeneratorDialog::values.contains(key);
    ValueSetGeneratorStaticValues values = ValueSetGeneratorDialog::values[key];

    this->funcComboBox = new QComboBox;
    for (uint i=0;i<uint(FUNC_N_FUNCS);i++)
    {
        this->funcComboBox->addItem(getFuncName(FuncType(i),this->valueTable.getKeyName()));
    }
    formLayout->addRow(this->valueTable.getValueName() + " = ",this->funcComboBox);

    this->aLineEdit = new ValueLineEdit;
    this->aLineEdit->setValue(values.a);
    if (isNew && this->valueTable.size() > 0)
    {
        this->aLineEdit->setValue(this->valueTable.getValue(0));
    }
    formLayout->addRow("A = ", this->aLineEdit);

    this->bLineEdit = new ValueLineEdit;
    this->bLineEdit->setValue(values.b);
    formLayout->addRow("B = ",this->bLineEdit);

    this->cLineEdit = new ValueLineEdit;
    this->cLineEdit->setValue(values.c);
    formLayout->addRow("C = ",this->cLineEdit);

    this->dLineEdit = new ValueLineEdit;
    this->dLineEdit->setValue(values.d);
    formLayout->addRow("D = ",this->dLineEdit);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    mainLayout->addWidget(buttonBox);

    QPushButton *cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    buttonBox->addButton(cancelButton,QDialogButtonBox::RejectRole);

    QPushButton *okButton = new QPushButton(okIcon, tr("Ok"));
    buttonBox->addButton(okButton,QDialogButtonBox::AcceptRole);

    QObject::connect(buttonBox,&QDialogButtonBox::rejected,this,&QDialog::reject);
    QObject::connect(buttonBox,&QDialogButtonBox::accepted,this,&QDialog::accept);

    QObject::connect(this->funcComboBox,&QComboBox::currentIndexChanged,this,&ValueSetGeneratorDialog::onFuncComboCurrentIndexChanged);

    this->funcComboBox->setCurrentIndex(values.funcType);
}

int ValueSetGeneratorDialog::exec()
{
    int retVal = QDialog::exec();

    if (retVal == QDialog::Accepted)
    {
        FuncType funcType = FuncType(this->funcComboBox->currentIndex());

        ValueSetGeneratorStaticValues values;

        values.funcType = FuncType(this->funcComboBox->currentIndex());
        values.a = this->aLineEdit->getValue();
        values.b = this->bLineEdit->getValue();
        values.c = this->cLineEdit->getValue();
        values.d = this->dLineEdit->getValue();
        for (uint i=0;i<this->valueTable.size();i++)
        {
            double key = this->valueTable.getKey(i);
            this->valueTable.add(key,funcFindValue(funcType,values,key));
        }

        ValueSetGeneratorDialog::values[this->valueTable.getKeyName() + "/" + this->valueTable.getValueName()] = values;
    }

    return retVal;
}

void ValueSetGeneratorDialog::onFuncComboCurrentIndexChanged(int index)
{
    this->aLineEdit->setEnabled(true);
    this->bLineEdit->setEnabled(true);
    this->cLineEdit->setEnabled(true);
    this->dLineEdit->setEnabled(true);

    switch (index)
    {
        case FUNC_FILL_A:
            this->bLineEdit->setDisabled(true);
            this->cLineEdit->setDisabled(true);
            this->dLineEdit->setDisabled(true);
            break;
        case FUNC_LINEAR:
            this->cLineEdit->setDisabled(true);
            this->dLineEdit->setDisabled(true);
            break;
        default:
            break;
    }
}
