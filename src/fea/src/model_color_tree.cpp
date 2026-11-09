#include <QPainter>
#include <QStyledItemDelegate>

#include "model_color_tree.h"
#include "application.h"

// Space at the top of a model row taken by the line in the color of the model and its padding.
static const int modelLineHeight = 2;
static const int modelLinePadding = 1;
static const int modelLineSpace = modelLineHeight + 2*modelLinePadding;
// Space separating a model from the model above it.
static const int modelSeparation = 2;

// Height of space above the line in row of a model.
static int findModelSeparation(const QModelIndex &index)
{
    return index.row() > 0 ? modelSeparation : 0;
}

// Height of space at the top of a row taken by what separates models.
static int findRowTopSpace(const QModelIndex &index)
{
    return index.parent().isValid() ? 0 : findModelSeparation(index) + modelLineSpace;
}

class ModelColorTreeItemDelegate : public QStyledItemDelegate
{

    public:

        using QStyledItemDelegate::QStyledItemDelegate;

        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
        {
            const QSize size = QStyledItemDelegate::sizeHint(option,index);
            return size + QSize(0,findRowTopSpace(index));
        }

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
        {
            QStyleOptionViewItem itemOption(option);
            itemOption.rect.setTop(itemOption.rect.top() + findRowTopSpace(index));
            QStyledItemDelegate::paint(painter,itemOption,index);
        }
};

ModelColorTree::ModelColorTree(QWidget *parent)
    : QTreeWidget{parent}
{
    this->setItemDelegate(new ModelColorTreeItemDelegate(this));
}

void ModelColorTree::drawRow(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const
{
    QTreeWidget::drawRow(painter,options,index);

    if (!index.parent().isValid())
    {
        uint modelID = this->findRowModelID(index);
        // Selection of the row must not show in the padding around the line.
        painter->fillRect(QRect(0,options.rect.top(),this->viewport()->width(),findRowTopSpace(index)),this->palette().color(QPalette::Base));
        painter->fillRect(QRect(0,options.rect.top()+findModelSeparation(index)+modelLinePadding,this->viewport()->width(),modelLineHeight),Application::instance()->getSession()->getModelColor(modelID));
    }
}

void ModelColorTree::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    QTreeWidget::drawBranches(painter,rect.adjusted(0,findRowTopSpace(index),0,0),index);
}
