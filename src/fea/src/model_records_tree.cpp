#include <QMap>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QScrollBar>

#include "model_records_tree.h"
#include "application.h"

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
            if (this->modelID < rModelRecordsTreeRecordID.modelID)
            {
                return true;
            }
            if (this->isRecord < rModelRecordsTreeRecordID.isRecord)
            {
                return true;
            }
            if (this->recordNumber < rModelRecordsTreeRecordID.recordNumber)
            {
                return true;
            }
            return false;
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
    this->setAutoScroll(false);
    QTreeWidgetItem* headerItem = new QTreeWidgetItem();
    headerItem->setText(ModelRecordsTree::RecordNumber,QString(tr("Record")));
    headerItem->setText(ModelRecordsTree::RecordFileName,QString(tr("File name")));
    headerItem->setIcon(ModelRecordsTree::Marked,QIcon(":/icons/media/pixmaps/range-play_play.svg"));
    this->setHeaderItem(headerItem);

    this->setColumnHidden(ModelRecordsTree::ModelID,true);
    this->setColumnHidden(ModelRecordsTree::IsRecord,true);
    this->setColumnHidden(ModelRecordsTree::PathFileName,true);

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
                uint recordNumber = 0;
                if (rModel.getTimeSolver().getEnabled())
                {
                    recordNumber = rModel.getTimeSolver().getCurrentTimeStep();
                }
                else
                {
                    if (rModel.getProblemTaskTree().getProblemTypeMask() & R_PROBLEM_STRESS_MODAL)
                    {
                        recordNumber = rModel.getProblemSetup().getModalSetup().getMode();
                    }
                }
                if (Application::instance()->getSession()->isModelSelected(i) && recordNumber == uint(j))
                {
                    childItem->setIcon(ModelRecordsTree::Marked,QIcon(":/icons/media/pixmaps/range-play_play.svg"));
                    childItem->setData(ModelRecordsTree::Marked,Qt::UserRole,QVariant(true));
                }
                else
                {
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
    this->resizeColumnToContents(ModelRecordsTree::Marked);

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
                    (*it)->setData(ModelRecordsTree::Marked,Qt::UserRole,QVariant(false));
                    prevItem->setData(ModelRecordsTree::Marked,Qt::UserRole,QVariant(true));
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
                if ((*it)->data(ModelRecordsTree::ModelID,Qt::UserRole).toUInt() == modelID)
                {
                    prevItem->setData(ModelRecordsTree::Marked,Qt::UserRole,false);
                    (*it)->setData(ModelRecordsTree::Marked,Qt::UserRole,true);
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
                (*it)->setData(ModelRecordsTree::Marked,Qt::UserRole,isFirst);
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
            lastItem->setData(ModelRecordsTree::Marked,Qt::UserRole,true);

            QTreeWidgetItemIterator it(this);
            while (*it)
            {
                if ((*it)->data(ModelRecordsTree::ModelID,Qt::UserRole).toUInt() == modelID)
                {
                    itemID2++;
                    if (itemID1 != itemID2)
                    {
                        (*it)->setData(ModelRecordsTree::Marked,Qt::UserRole,false);
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
                (*it)->setIcon(ModelRecordsTree::Marked,QIcon(":/icons/media/pixmaps/range-play_play.svg"));
            }
            else
            {
                (*it)->setData(ModelRecordsTree::Marked,Qt::DecorationRole,QVariant());
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
        item->setIcon(ModelRecordsTree::Marked,QIcon(":/icons/media/pixmaps/range-play_play.svg"));
        this->setCurrentItem(item);
        this->scrollToItem(item);
        recordMarkedIndicator = true;
    }
    else
    {
        item->setData(ModelRecordsTree::Marked,Qt::DecorationRole,QVariant());
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
            (*it)->setData(ModelRecordsTree::Marked,Qt::UserRole,false);
        }
        ++it;
    }
    if (!item->data(ModelRecordsTree::Marked,Qt::UserRole).toBool())
    {
        item->setData(ModelRecordsTree::Marked,Qt::UserRole,true);
    }
}
