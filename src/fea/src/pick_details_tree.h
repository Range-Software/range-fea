#ifndef PICK_DETAILS_TREE_H
#define PICK_DETAILS_TREE_H

#include <QTreeWidget>

class PickDetailsTree : public QTreeWidget
{

    Q_OBJECT

    public:

        //! Constructor.
        explicit PickDetailsTree(QWidget *parent = nullptr);

    protected:

        //! Populate the tree.
        void populate();
        
    protected slots:

        //! Pick list has changed.
        void onPickListChanged();

        //! Tree item wwas expanded.
        void onItemExpanded(QTreeWidgetItem *);

        //! Resize columns to content.
        void resizeColumnsToContent();
        
};

#endif // PICK_DETAILS_TREE_H
