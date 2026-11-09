#ifndef MODEL_COLOR_TREE_H
#define MODEL_COLOR_TREE_H

#include <QTreeWidget>

//! Tree with models as top level items, models are separated by a line in the color of the model.
class ModelColorTree : public QTreeWidget
{

    Q_OBJECT

    public:

        //! Constructor.
        explicit ModelColorTree(QWidget *parent = nullptr);

    protected:

        //! Return ID of the model in given top level row.
        virtual uint findRowModelID(const QModelIndex &index) const = 0;

        //! Draw row, row of a model has a line in the color of the model at its top.
        void drawRow(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const override;

        //! Draw branches, branches of a model are moved below the line in the color of the model.
        void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const override;

};

#endif // MODEL_COLOR_TREE_H
