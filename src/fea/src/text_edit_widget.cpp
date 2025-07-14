#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFontDatabase>

#include "text_edit_widget.h"

TextEditWidget::TextEditWidget(QWidget *parent)
    : QWidget(parent)
{
    QIcon cancelIcon(":/icons/action/pixmaps/range-cancel.svg");
    QIcon okIcon(":/icons/action/pixmaps/range-ok.svg");

    QVBoxLayout *layout = new QVBoxLayout;
    this->setLayout(layout);

    this->textEdit = new QPlainTextEdit;
    layout->addWidget(this->textEdit);

    this->textEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    QObject::connect(this->textEdit,&QPlainTextEdit::textChanged,this,&TextEditWidget::onTextEditChanged);

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch(1);
    layout->addLayout(buttonsLayout);

    this->cancelButton = new QPushButton(cancelIcon, tr("Cancel"));
    buttonsLayout->addWidget(this->cancelButton);

    QObject::connect(this->cancelButton,&QPushButton::clicked,this,&TextEditWidget::onCancelButtonClicked);

    this->okButton = new QPushButton(okIcon, tr("Ok"));
    this->okButton->setDisabled(true);
    buttonsLayout->addWidget(this->okButton);

    QObject::connect(this->okButton,&QPushButton::clicked,this,&TextEditWidget::onOkButtonClicked);
}

void TextEditWidget::setText(const QString &text)
{
    this->textEdit->setPlainText(text);
}

void TextEditWidget::onTextEditChanged()
{
    this->okButton->setDisabled(this->textEdit->toPlainText().isEmpty());
}

void TextEditWidget::onCancelButtonClicked()
{
    emit this->closed();
}

void TextEditWidget::onOkButtonClicked()
{
    emit this->changed(textEdit->toPlainText());
    emit this->closed();
}
