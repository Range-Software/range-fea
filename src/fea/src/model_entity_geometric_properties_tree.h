#ifndef MODEL_ENTITY_GEOMETRIC_PROPERTIES_TREE_H
#define MODEL_ENTITY_GEOMETRIC_PROPERTIES_TREE_H

#include <QTreeWidget>

class ModelEntityGeometricPropertiesTree : public QTreeWidget
{

    Q_OBJECT

    public:

        //! Constructor.
        explicit ModelEntityGeometricPropertiesTree(QWidget *parent = nullptr);

    protected:

        //! Populate model.
        void populate();

    private slots:

        //! Model selection has changed.
        void onModelSelectionChanged(uint);

        //! Item data has changed.
        void onItemChanged(QTreeWidgetItem *item, int column);

        //! Catch double click signal.
        void onItemDoubleClicked(QTreeWidgetItem *item, int column);
        
};

#endif // MODEL_ENTITY_GEOMETRIC_PROPERTIES_TREE_H
