#ifndef DOCUMENT_TREE_H
#define DOCUMENT_TREE_H

#include <QTreeWidget>

class DocumentTree : public QTreeWidget
{

    Q_OBJECT

    public:

        //! Constructor.
        explicit DocumentTree(QWidget *parent = nullptr);

    private:

        //! Add document.
        void addDocument(QTreeWidgetItem *item, uint modelID, const QString &filename);
        
    protected slots:

        //! Populate.
        void populate();

        //! Update tree.
        void updateDocuments();

        //! Model has been added.
        void onModelAdded(uint modelID);

        //! Model has been changed.
        void onModelChanged(uint modelID);
        
        //! Model has been removed.
        void onModelRemoved(uint modelID);

        //! Catch tree-widget item doubleclick.
        void onItemDoubleClicked(QTreeWidgetItem *item, int column);

};

#endif // DOCUMENT_TREE_H
