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
        //! Lowest record number of the highlighted range (0 = no highlight).
        uint highlightFirst;
        //! Highest record number of the highlighted range.
        uint highlightLast;

    public:

        //! Constructor.
        explicit ModelRecordsTree(QWidget *parent = nullptr);

        //! Return true if marked record is first.
        bool isFirst();

        //! Return true if marked record is last.
        bool isLast();

        //! Return the lowest record number across selected models (0 if none).
        uint getFirstRecordNumber();

        //! Return the highest record number across selected models (0 if none).
        uint getLastRecordNumber();

        //! Return the currently marked record number (max across selected models, 0 if none).
        uint getMarkedRecordNumber();

        //! Return the next record number after the marked one (0 if marked is last).
        uint getNextRecordNumber();

        //! Return the number of records within [first,last] (max across selected models).
        uint getRecordCountInRange(uint first, uint last);

    private:

        //! Populate tree.
        void populate();

        //! Apply the background highlight to records inside the from-to range.
        void applyHighlightRange();

        //! Return the currently selected record items (model rows excluded).
        QList<QTreeWidgetItem*> selectedRecordItems() const;

        //! Mark the given record item so that it gets loaded.
        void loadRecordItem(QTreeWidgetItem *item);

    public slots:

        //! Set the highlighted record range and refresh row backgrounds.
        void setHighlightRange(uint first, uint last);

    signals:

        //! Record marked.
        void recordMarked(uint modelID, const QString &recordFileName);

        //! Record count may have changed (tree repopulated or selection changed).
        void recordCountChanged();

        //! Request setting the first record of the play / record range.
        void setFromRequested(uint recordNumber);

        //! Request setting the last record of the play / record range.
        void setToRequested(uint recordNumber);

        //! Request setting both ends of the play / record range.
        void setRangeRequested(uint first, uint last);

    public slots:

        //! Forced mark current record.
        void markCurrent();

        //! Mark the record with the given record number.
        void markRecord(uint recordNumber);

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

        //! Show the context menu at the given position.
        void onContextMenu(const QPoint &position);

        //! Remove the selected record files from disk.
        void removeSelectedRecords();

};

#endif // MODEL_RECORDS_TREE_H
