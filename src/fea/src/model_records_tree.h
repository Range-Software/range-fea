#ifndef MODEL_RECORDS_TREE_H
#define MODEL_RECORDS_TREE_H

#include <QTreeWidget>

class ModelRecordsTree : public QTreeWidget
{

    Q_OBJECT

    protected:

        enum ModelRecordsTreeColumn
        {
            Marked = 0,
            ModelID,
            IsRecord,
            RecordNumber,
            RecordFileName,
            PathFileName,
            NColumns
        };

        bool directoryChanged;

    public:

        //! Constructor.
        explicit ModelRecordsTree(QWidget *parent = nullptr);

        //! Return true if marked record is first.
        bool isFirst();

        //! Return true if marked record is last.
        bool isLast();

    private:

        //! Populate tree.
        void populate();

    signals:

        //! Record marked.
        void recordMarked(uint modelID, const QString &recordFileName);

    public slots:

        //! Forced mark current record.
        void markCurrent();

        //! Mark previous record.
        void markPrevious();

        //! Mark next record.
        void markNext();

        //! Mark first record.
        void markFirst();

        //! Mark last record.
        void markLast();
        
    protected slots:

        //! Timeout expired.
        void onTimeout();

        //! Directory has changed.
        void onDirectoryChanged(const QString &);

        //! Model selection has changed.
        void onModelSelectionChanged(uint modelID);

        //! Model havs changed.
        void onModelChanged(uint);

        //! On item changed.
        void onItemChanged(QTreeWidgetItem *item, int column);

        //! On item activated.
        void onItemActivated(QTreeWidgetItem *item, int);
        
};

#endif // MODEL_RECORDS_TREE_H
