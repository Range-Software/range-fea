#include <QMap>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QHeaderView>
#include <QScrollBar>

#include "model_records_tree.h"
#include "application.h"

namespace
{

const char *MarkedRecordIcon = ":/icons/media/pixmaps/range-play_play.svg";
const int MarkedRecordColumnWidth = 48;

void setMarkedRecordIcon(QTreeWidgetItem *item, int column, bool marked)
{
    item->setIcon(column,marked ? QIcon(MarkedRecordIcon) : QIcon());
}

void setMarkedRecord(QTreeWidgetItem *item, int column, bool marked, bool modelSelected)
{
    item->setData(column,Qt::UserRole,QVariant(marked));
    // Only update the icon directly when the tree's signals are blocked (e.g. inside
    // populate()).  When signals are live, the setData call above already triggers
    // onItemChanged which sets the icon — calling setMarkedRecordIcon here as well
    // would fire onItemChanged a second time via setIcon → itemChanged, causing a
    // duplicate recordMarked emission and a second async load job that resets state.
    QTreeWidget *tree = item->treeWidget();
    if (!tree || tree->signalsBlocked())
    {
        setMarkedRecordIcon(item,column,marked && modelSelected);
    }
}

}

class ModelRecordsTreeRecordID
{
    public:
        ModelRecordsTreeRecordID(uint modelID,bool isRecord,uint recordNumber)
            : modelID(modelID)
            , isRecord(isRecord)
            , recordNumber(recordNumber)
        {
        }
        bool operator < (const ModelRecordsTreeRecordID &rModelRecordsTreeRecordID) const
        {
            if (this->modelID != rModelRecordsTreeRecordID.modelID)
            {
                return this->modelID < rModelRecordsTreeRecordID.modelID;
            }
            if (this->isRecord != rModelRecordsTreeRecordID.isRecord)
            {
                return this->isRecord < rModelRecordsTreeRecordID.isRecord;
            }
            return this->recordNumber < rModelRecordsTreeRecordID.recordNumber;
        }

    public:
        uint modelID;
        bool isRecord;
        uint recordNumber;
};

ModelRecordsTree::ModelRecordsTree(QWidget *parent)
    : QTreeWidget(parent)
    , directoryChanged(false)
{
    R_LOG_TRACE;
    this->setColumnCount(ModelRecordsTree::NColumns);
    this->setSelectionMode(QAbstractItemView::SingleSelection);
    this->setRootIsDecorated(false);
    // this->setIndentation(0);
    this->setAutoScroll(false);
    QTreeWidgetItem* headerItem = new QTreeWidgetItem();
    headerItem->setText(ModelRecordsTree::RecordNumber,QString(tr("Record")));
    headerItem->setText(ModelRecordsTree::RecordFileName,QString(tr("File name")));
    headerItem->setIcon(ModelRecordsTree::Marked,QIcon(MarkedRecordIcon));
    this->setHeaderItem(headerItem);

    this->setColumnHidden(ModelRecordsTree::ModelID,true);
    this->setColumnHidden(ModelRecordsTree::IsRecord,true);
    this->setColumnHidden(ModelRecordsTree::PathFileName,true);
    this->header()->setSectionResizeMode(ModelRecordsTree::Marked,QHeaderView::Fixed);
    this->setColumnWidth(ModelRecordsTree::Marked,MarkedRecordColumnWidth);

    this->populate();

    QObject::connect(Application::instance()->getSession(),
                     &Session::modelSelectionChanged,
                     this,
                     &ModelRecordsTree::onModelSelectionChanged);
    QObject::connect(Application::instance()->getSession(),
                     &Session::modelAdded,
                     this,
                     &ModelRecordsTree::onModelChanged);
    QObject::connect(Application::instance()->getSession(),
                     &Session::modelChanged,
                     this,
                     &ModelRecordsTree::onModelChanged);
    QObject::connect(Application::instance()->getSession(),
                     &Session::modelRemoved,
                     this,
                     &ModelRecordsTree::onModelChanged);
    QObject::connect(this,
                     &ModelRecordsTree::itemChanged,
                     this,
                     &ModelRecordsTree::onItemChanged);
    QObject::connect(this,
                     &ModelRecordsTree::itemActivated,
                     this,
                     &ModelRecordsTree::onItemActivated);

    QTimer *timer = new QTimer(this);
    QObject::connect(timer,
                     &QTimer::timeout,
                     this,
                     &ModelRecordsTree::onTimeout);
    timer->start(1500);

    QFileSystemWatcher *directoryWatcher = new QFileSystemWatcher(this);
    directoryWatcher->addPath(Application::instance()->getApplicationSettings()->getDataDir());

    QObject::connect(directoryWatcher,
                     &QFileSystemWatcher::directoryChanged,
                     this,
                     &ModelRecordsTree::onDirectoryChanged);
}

bool ModelRecordsTree::isFirst()
{
    R_LOG_TRACE;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        uint modelID = modelIDs[i];

        QTreeWidgetItemIterator it(this);
        while (*it)
        {
            if ((*it)->data(ModelRecordsTree::IsRecord,Qt::UserRole).toBool() &&
                (*it)->data(ModelRecordsTree::ModelID,Qt::UserRole).toUInt() == modelID)
            {
                if ((*it)->data(ModelRecordsTree::Marked,Qt::UserRole).toBool())
                {
                    return true;
                }
                break;
            }
            ++it;
        }
    }
    return false;
}

bool ModelRecordsTree::isLast()
{
    R_LOG_TRACE;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        uint modelID = modelIDs[i];
        bool marked = false;

        QTreeWidgetItemIterator it(this);
        while (*it)
        {
            if ((*it)->data(ModelRecordsTree::IsRecord,Qt::UserRole).toBool() &&
                (*it)->data(ModelRecordsTree::ModelID,Qt::UserRole).toUInt() == modelID)
            {
                if ((*it)->data(ModelRecordsTree::Marked,Qt::UserRole).toBool())
                {
                    marked = true;
                }
                else
                {
                    if (marked)
                    {
                        return false;
                    }
                }
            }
            ++it;
        }
    }
    return true;
}

void ModelRecordsTree::populate()
{
    R_LOG_TRACE;
    if (!Application::instance()->getSession()->trylock())
    {
        return;
    }

    QList<QTreeWidgetItem*> selectedItemList = this->selectedItems();

    QMap<ModelRecordsTreeRecordID,bool> selectedIDs;

    for (int i=0;i<selectedItemList.size();i++)
    {
        ModelRecordsTreeRecordID recordID(selectedItemList.at(i)->data(ModelRecordsTree::ModelID,Qt::UserRole).toUInt(),
                                          selectedItemList.at(i)->data(ModelRecordsTree::IsRecord,Qt::UserRole).toBool(),
                                          selectedItemList.at(i)->data(ModelRecordsTree::RecordNumber,Qt::DisplayRole).toUInt());
        selectedIDs[recordID] = true;
    }

    // Preserve the marked (triangle) state across the clear/rebuild cycle.
    // The mark is set synchronously by onItemChanged when a record is activated,
    // but the file load is async — populate() may be called before getCurrentTimeStep()
    // is updated, causing the mark to land on the wrong record.  Using the pre-clear
    // state as the source of truth avoids that race.  getCurrentTimeStep() is still
    // used as a fallback for the initial population when nothing is yet marked.
    QMap<ModelRecordsTreeRecordID,bool> markedIDs;
    {
        QTreeWidgetItemIterator it(this);
        while (*it)
        {
            if ((*it)->data(ModelRecordsTree::IsRecord,Qt::UserRole).toBool() &&
                (*it)->data(ModelRecordsTree::Marked,Qt::UserRole).toBool())
            {
                ModelRecordsTreeRecordID recordID((*it)->data(ModelRecordsTree::ModelID,Qt::UserRole).toUInt(),
                                                  true,
                                                  (*it)->data(ModelRecordsTree::RecordNumber,Qt::DisplayRole).toUInt());
                markedIDs[recordID] = true;
            }
            ++it;
        }
    }

    uint nModels = Application::instance()->getSession()->getNModels();

    this->blockSignals(true);

    QScrollBar *vScrollBar = this->verticalScrollBar();
    int vScrollValue = vScrollBar->value();
    this->clear();

    for (uint i=0;i<nModels;i++)
    {
        const Model &rModel = Application::instance()->getSession()->getModel(i);

        QTreeWidgetItem *item = new QTreeWidgetItem(this);
        item->setFirstColumnSpanned(true);
        item->setData(ModelRecordsTree::ModelID,Qt::UserRole,QVariant(i));
        item->setData(ModelRecordsTree::IsRecord,Qt::UserRole,QVariant(false));
        item->setData(ModelRecordsTree::Marked,Qt::UserRole,QVariant(false));
        item->setText(ModelRecordsTree::Marked,rModel.getName());

        if (selectedIDs.constFind(ModelRecordsTreeRecordID(i,false,0)) != selectedIDs.constEnd())
        {
            this->setCurrentItem(item);
        }

        QList<QString> recordFiles = rModel.getRecordFiles(false);

        for (int j=0;j<recordFiles.size();j++)
        {
            if (QFile::exists(recordFiles[j]))
            {
                QFileInfo fi(recordFiles[j]);
                QString elipsizeFileName = fi.baseName();

                QTreeWidgetItem *childItem = new QTreeWidgetItem(item);
                childItem->setData(ModelRecordsTree::ModelID,Qt::UserRole,QVariant(i));
                childItem->setData(ModelRecordsTree::IsRecord,Qt::UserRole,QVariant(true));
                childItem->setData(ModelRecordsTree::RecordNumber,Qt::DisplayRole,QVariant(j+1));
                childItem->setText(ModelRecordsTree::RecordFileName,elipsizeFileName);
                childItem->setText(ModelRecordsTree::PathFileName,recordFiles[j]);
                bool isMarked = false;
                if (!markedIDs.isEmpty())
                {
                    // Prefer the pre-clear marked state to avoid the async-load race.
                    isMarked = markedIDs.contains(ModelRecordsTreeRecordID(i,true,uint(j+1)));
                }
                else
                {
                    // Nothing was marked yet (initial population): fall back to the
                    // time solver's current step.
                    uint recordNumber = 0;
                    if (rModel.getTimeSolver().getEnabled())
                    {
                        recordNumber = rModel.getTimeSolver().getCurrentTimeStep() + 1;
                    }
                    else
                    {
                        if (rModel.getProblemTaskTree().getProblemTypeMask() & R_PROBLEM_STRESS_MODAL)
                        {
                            recordNumber = rModel.getProblemSetup().getModalSetup().getMode() + 1;
                        }
                    }
                    isMarked = Application::instance()->getSession()->isModelSelected(i) && recordNumber == uint(j+1);
                }
                if (isMarked)
                {
                    setMarkedRecordIcon(childItem,ModelRecordsTree::Marked,Application::instance()->getSession()->isModelSelected(i));
                    childItem->setData(ModelRecordsTree::Marked,Qt::UserRole,QVariant(true));
                }
                else
                {
                    setMarkedRecordIcon(childItem,ModelRecordsTree::Marked,false);
                    childItem->setData(ModelRecordsTree::Marked,Qt::UserRole,QVariant(false));
                }

                if (selectedIDs.constFind(ModelRecordsTreeRecordID(i,true,j+1)) != selectedIDs.constEnd())
                {
                    this->setCurrentItem(childItem);
                }
            }
        }
        item->setExpanded(true);
    }

    this->resizeColumnToContents(ModelRecordsTree::RecordNumber);
    this->resizeColumnToContents(ModelRecordsTree::RecordFileName);
    this->setColumnWidth(ModelRecordsTree::Marked,MarkedRecordColumnWidth);

    vScrollBar->setValue(vScrollValue);

    this->blockSignals(false);

    Application::instance()->getSession()->unlock();
}

void ModelRecordsTree::markCurrent()
{
    R_LOG_TRACE;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        uint modelID = modelIDs[i];

        QTreeWidgetItemIterator it(this);
        while (*it)
        {
            if ((*it)->data(ModelRecordsTree::IsRecord,Qt::UserRole).toBool() &&
                (*it)->data(ModelRecordsTree::ModelID,Qt::UserRole).toUInt() == modelID)
            {
                if ((*it)->data(ModelRecordsTree::Marked,Qt::UserRole).toBool())
                {
                    emit this->recordMarked((*it)->data(ModelRecordsTree::ModelID,Qt::UserRole).toUInt(),
                                            (*it)->text(ModelRecordsTree::PathFileName));
                }
            }
            ++it;
        }
    }
}

void ModelRecordsTree::markPrevious()
{
    R_LOG_TRACE;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        uint modelID = modelIDs[i];

        QTreeWidgetItem *prevItem = nullptr;

        QTreeWidgetItemIterator it(this);
        while (*it)
        {
            if ((*it)->data(ModelRecordsTree::Marked,Qt::UserRole).toBool() &&
                (*it)->data(ModelRecordsTree::ModelID,Qt::UserRole).toUInt() == modelID)
            {
                if (prevItem)
                {
                    setMarkedRecord(*it,ModelRecordsTree::Marked,false,false);
                    setMarkedRecord(prevItem,ModelRecordsTree::Marked,true,true);
                }
                break;
            }
            if ((*it)->data(ModelRecordsTree::IsRecord,Qt::UserRole).toBool() &&
                (*it)->data(ModelRecordsTree::ModelID,Qt::UserRole).toUInt() == modelID)
            {
                prevItem = (*it);
            }
            ++it;
        }
    }
}

void ModelRecordsTree::markNext()
{
    R_LOG_TRACE;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        uint modelID = modelIDs[i];

        bool markItem = false;

        QTreeWidgetItem *prevItem = nullptr;

        QTreeWidgetItemIterator it(this);
        while (*it)
        {
            if (markItem)
            {
                if ((*it)->data(ModelRecordsTree::IsRecord,Qt::UserRole).toBool() &&
                    (*it)->data(ModelRecordsTree::ModelID,Qt::UserRole).toUInt() == modelID)
                {
                    setMarkedRecord(prevItem,ModelRecordsTree::Marked,false,false);
                    setMarkedRecord(*it,ModelRecordsTree::Marked,true,true);
                }
                break;
            }
            if ((*it)->data(ModelRecordsTree::Marked,Qt::UserRole).toBool() &&
                (*it)->data(ModelRecordsTree::ModelID,Qt::UserRole).toUInt() == modelID)
            {
                modelID = (*it)->data(ModelRecordsTree::ModelID,Qt::UserRole).toUInt();
                prevItem = (*it);
                markItem = true;
            }
            ++it;
        }
    }
}

void ModelRecordsTree::markFirst()
{
    R_LOG_TRACE;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        uint modelID = modelIDs[i];
        bool isFirst = true;

        QTreeWidgetItemIterator it(this);
        while (*it)
        {
            if ((*it)->data(ModelRecordsTree::ModelID,Qt::UserRole).toUInt() == modelID &&
                (*it)->data(ModelRecordsTree::IsRecord,Qt::UserRole).toBool())
            {
                setMarkedRecord(*it,ModelRecordsTree::Marked,isFirst,isFirst);
                isFirst = false;
            }
            ++it;
        }
    }
}

void ModelRecordsTree::markLast()
{
    R_LOG_TRACE;
    QList<uint> modelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    for (int i=0;i<modelIDs.size();i++)
    {
        uint modelID = modelIDs[i];
        uint itemID1 = 0;
        uint itemID2 = 0;
        QTreeWidgetItem *lastItem = nullptr;

        QTreeWidgetItemIterator it(this);
        while (*it)
        {
            if ((*it)->data(ModelRecordsTree::ModelID,Qt::UserRole).toUInt() == modelID)
            {
                lastItem = (*it);
                itemID1++;
            }
            ++it;
        }

        if (lastItem && !lastItem->data(ModelRecordsTree::Marked,Qt::UserRole).toBool())
        {
            setMarkedRecord(lastItem,ModelRecordsTree::Marked,true,true);

            QTreeWidgetItemIterator it(this);
            while (*it)
            {
                if ((*it)->data(ModelRecordsTree::ModelID,Qt::UserRole).toUInt() == modelID)
                {
                    itemID2++;
                    if (itemID1 != itemID2)
                    {
                        setMarkedRecord(*it,ModelRecordsTree::Marked,false,false);
                    }
                }
                ++it;
            }
        }
    }
}

void ModelRecordsTree::onTimeout()
{
    if (this->directoryChanged)
    {
        this->directoryChanged = false;
        this->populate();
    }
}

void ModelRecordsTree::onDirectoryChanged(const QString &)
{
    this->directoryChanged = true;
}

void ModelRecordsTree::onModelSelectionChanged(uint modelID)
{
    R_LOG_TRACE;
    this->blockSignals(true);
    QTreeWidgetItemIterator it(this);
    while (*it)
    {
        if ((*it)->data(ModelRecordsTree::IsRecord,Qt::UserRole).toBool() &&
            (*it)->data(ModelRecordsTree::Marked,Qt::UserRole).toBool() &&
            (*it)->data(ModelRecordsTree::ModelID,Qt::UserRole).toUInt() == modelID)
        {
            if (Application::instance()->getSession()->isModelSelected(modelID))
            {
                setMarkedRecordIcon(*it,ModelRecordsTree::Marked,true);
            }
            else
            {
                setMarkedRecordIcon(*it,ModelRecordsTree::Marked,false);
            }
        }
        ++it;
    }
    this->blockSignals(false);
}

void ModelRecordsTree::onModelChanged(uint)
{
    R_LOG_TRACE;
    this->populate();
}

void ModelRecordsTree::onItemChanged(QTreeWidgetItem *item, int column)
{
    R_LOG_TRACE;
    if (column != ModelRecordsTree::Marked)
    {
        return;
    }

    bool recordMarkedIndicator = false;

    this->blockSignals(true);

    if (item->data(ModelRecordsTree::Marked,Qt::UserRole).toBool() &&
        Application::instance()->getSession()->isModelSelected(item->data(ModelRecordsTree::ModelID,Qt::UserRole).toUInt()))
    {
        setMarkedRecordIcon(item,ModelRecordsTree::Marked,true);
        this->setCurrentItem(item);
        this->scrollToItem(item);
        recordMarkedIndicator = true;
    }
    else
    {
        setMarkedRecordIcon(item,ModelRecordsTree::Marked,false);
    }

    this->blockSignals(false);

    if (recordMarkedIndicator)
    {
        emit this->recordMarked(item->data(ModelRecordsTree::ModelID,Qt::UserRole).toUInt(),
                                item->text(ModelRecordsTree::PathFileName));
    }
}

void ModelRecordsTree::onItemActivated(QTreeWidgetItem *item, int)
{
    R_LOG_TRACE;
    uint modelID = item->data(ModelRecordsTree::ModelID,Qt::UserRole).toUInt();

    QTreeWidgetItemIterator it(this);
    while (*it)
    {
        if ((*it)->data(ModelRecordsTree::IsRecord,Qt::UserRole).toBool() &&
            (*it)->data(ModelRecordsTree::Marked,Qt::UserRole).toBool() &&
            (*it)->data(ModelRecordsTree::ModelID,Qt::UserRole).toUInt() == modelID)
        {
            setMarkedRecord(*it,ModelRecordsTree::Marked,false,false);
        }
        ++it;
    }
    if (!item->data(ModelRecordsTree::Marked,Qt::UserRole).toBool())
    {
        setMarkedRecord(item,
                        ModelRecordsTree::Marked,
                        true,
                        Application::instance()->getSession()->isModelSelected(modelID));
    }
}
