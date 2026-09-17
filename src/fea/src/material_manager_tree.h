#ifndef MATERIAL_MANAGER_TREE_H
#define MATERIAL_MANAGER_TREE_H

#include <QTreeWidget>
#include <QPushButton>
#include <QMap>

#include <rml_material.h>

class MaterialManagerTree : public QWidget
{
    Q_OBJECT

    protected:

        enum ColumnType
        {
            ColumnName = 0,
            ColumnFile,
            NunberOfColumns
        };

        //! Problem type mask.
        RProblemTypeMask problemTypeMask;

        //! Materials of the tree items, by file path. An item carries only the
        //! name and the file path of its material, so without this every
        //! material has to be read from its file again whenever an item has to
        //! be matched against the problem type - which happens on every model
        //! selection and problem change. Rebuilt by onDirectoryChanged().
        QMap<QString,RMaterial> materialCache;

        QTreeWidget *treeWidget;

        QPushButton *newButton;
        QPushButton *deleteButton;

    public:

        //! Constructor.
        explicit MaterialManagerTree(QWidget *parent = nullptr);

    protected:

        //! Update check states
        void updateCheckStates();

        //! Update problem type mask.
        void updateProblemTypeMask();

        //! Set item valid.
        void setItemValid(QTreeWidgetItem *item, bool valid);

        //! Update item with material to the tree.
        void updateItem(QTreeWidgetItem *item, const RMaterial &material, const QString &filePath, bool setSelected);

        //! Return material belonging to given item.
        //! The material is read from its file and cached if it is not cached yet.
        //! If ok is given it is set to false when the material could not be read.
        RMaterial findItemMaterial(QTreeWidgetItem *item, bool *ok = nullptr);

        //! Return file names for given name.
        QStringList findFiles(const QString &name) const;

        //! Return name that does not already exist.
        QString findAvailableName(const QString &name) const;

        //! Compare material to materials in tree and return 0 if same, 1 if different, -1 if unknown.
        int compareModelMaterial(const RMaterial &material) const;

        //! Build file path for given material.
        static QString buildFilePath(const RMaterial &material);

        //! Read material from file.
        //! If the file is not stored in the default format it is converted and
        //! the former file is removed. In such case filePath is updated to
        //! point to the converted file.
        static RMaterial read(QString &filePath);

        //! Write material to file.
        static void write(const QString &fileName, const RMaterial &material);

    public slots:

        //! Material has changed.
        void onMaterialChanged(const RMaterial &material);

    private slots:

        //! Material directory has changed.
        void onDirectoryChanged(const QString &path);

        //! Called on entity selection changed.
        void onModelSelectionChanged(uint);

        //! Called when new model is added to session.
        void onModelAdded(uint modelID);

        //! Called when problem is changed.
        void onProblemChanged(uint);

        //! Called when data has been changed.
        void onItemChanged(QTreeWidgetItem *item, int column);

        //! Catch doubleClicked signal.
        void onItemDoubleClicked(QTreeWidgetItem *item, int column);

        //! Catch itemSelectionChanged signal.
        void onItemSelectionChanged();

        //! New material button was clicked.
        void onMaterialNew();

        //! Delete material button was clicked.
        void onMaterialDelete();

    signals:

        //! Signal that material selection has changed.
        void materialSelected(const RMaterial &material);

};

#endif /* MATERIAL_MANAGER_TREE_H */
