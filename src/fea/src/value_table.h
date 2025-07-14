#ifndef VALUE_TABLE_H
#define VALUE_TABLE_H

#include <QTableWidget>
#include <QAction>

#include <rml_variable.h>

class ValueTable : public QTableWidget
{

    Q_OBJECT

    protected:

        //! Value data type.
        RVariableDataType dataType;
        //! Insert value action.
        QAction *insertValueAction;
        //! Delete value action.
        QAction *deleteValueAction;
        //! Import from file action.
        QAction *importFromFileAction;
        //! View graph action.
        QAction *viewGraphAction;
        //! View graph action.
        QAction *fillValuesAction;

    public:

        //! Constructor.
        explicit ValueTable(QWidget *parent = nullptr);

        //! Set value data type.
        void setDataType(RVariableDataType dataType);

        //! Set key header.
        void setKeyHeader(const QString &header);

        //! Set value header.
        void setValueHeader(const QString &header);

        //! Add value at given key.
        void addValue(double key, double value, int rowNumber = -1);

        //! Add value at given position.
        void removeValue(int rowNumber);

        //! Return key at given position.
        double getKey(uint row) const;

        //! Return value at given position.
        double getValue(uint row) const;

    signals:

        //! Size of the table has changed.
        void sizeChanged(uint nRows);

    private slots:

        //! Catch itamChanged signal.
        void onItemChanged(QTableWidgetItem *item);

        //! Insert value triggered.
        void onInsertValue();

        //! Delete value triggered.
        void onDeleteValue();

        //! Import from file triggered.
        void onImportFromFile();

        //! View graph triggered.
        void onViewGraph();

        //! Fill values triggered.
        void onFillValues();

};

#endif /* VALUE_TABLE_H */
