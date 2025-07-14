#include <QDesktopServices>
#include <QFileInfo>

#include "document_tree.h"
#include "application.h"

typedef enum _DocumentTreeColumn
{
    DOCUMENT_TREE_COLUMN_NAME = 0,
    DOCUMENT_TREE_COLUMN_FILENAME,
    DOCUMENT_TREE_COLUMN_MODEL_ID,
    DOCUMENT_TREE_COLUMN_IS_DOCUMENT,
    DOCUMENT_TREE_N_COLUMNS
} DocumentTreeColumn;

DocumentTree::DocumentTree(QWidget *parent) :
    QTreeWidget(parent)
{
    R_LOG_TRACE;
    this->setColumnCount(DOCUMENT_TREE_N_COLUMNS);
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);
    this->setRootIsDecorated(true);
    this->setAutoScroll(true);

    QTreeWidgetItem* headerItem = new QTreeWidgetItem();
    headerItem->setText(DOCUMENT_TREE_COLUMN_NAME,QString(tr("File name")));
    headerItem->setIcon(DOCUMENT_TREE_COLUMN_NAME,QIcon(":/icons/mime/pixmaps/range-file-document.svg"));
    this->setHeaderItem(headerItem);

    this->setColumnHidden(DOCUMENT_TREE_COLUMN_FILENAME,true);
    this->setColumnHidden(DOCUMENT_TREE_COLUMN_MODEL_ID,true);
    this->setColumnHidden(DOCUMENT_TREE_COLUMN_IS_DOCUMENT,true);

    this->sortByColumn(DOCUMENT_TREE_COLUMN_FILENAME,Qt::AscendingOrder);

    this->populate();

    QObject::connect(Application::instance()->getSession(),
                     &Session::modelAdded,
                     this,
                     &DocumentTree::onModelAdded);
    QObject::connect(Application::instance()->getSession(),
                     &Session::modelChanged,
                     this,
                     &DocumentTree::onModelChanged);
    QObject::connect(Application::instance()->getSession(),
                     &Session::modelRenamed,
                     this,
                     &DocumentTree::onModelChanged);
    QObject::connect(Application::instance()->getSession(),
                     &Session::modelRemoved,
                     this,
                     &DocumentTree::onModelRemoved);

    QObject::connect(this,
                     &QTreeWidget::itemDoubleClicked,
                     this,
                     &DocumentTree::onItemDoubleClicked);

    QTimer *timer = new QTimer(this);
    QObject::connect(timer,
                     &QTimer::timeout,
                     this,
                     &DocumentTree::updateDocuments);
    timer->start(1000);
}

void DocumentTree::addDocument(QTreeWidgetItem *item, uint modelID, const QString &filename)
{
    R_LOG_TRACE;
    QFileInfo fi(filename);

    QTreeWidgetItem *childItem = new QTreeWidgetItem(item);

    childItem->setText(DOCUMENT_TREE_COLUMN_NAME,fi.fileName());
    childItem->setText(DOCUMENT_TREE_COLUMN_FILENAME,filename);
    childItem->setData(DOCUMENT_TREE_COLUMN_MODEL_ID,Qt::UserRole,QVariant(modelID));
    childItem->setData(DOCUMENT_TREE_COLUMN_IS_DOCUMENT,Qt::UserRole,QVariant(true));
}

void DocumentTree::populate()
{
    R_LOG_TRACE;
    this->blockSignals(true);
    this->clear();
    this->blockSignals(false);

    for (uint i=0;i<Application::instance()->getSession()->getNModels();i++)
    {
        this->onModelAdded(i);
    }
}

void DocumentTree::updateDocuments()
{
    R_LOG_TRACE;
    if (!Application::instance()->getSession()->trylock())
    {
        return;
    }

    this->blockSignals(true);

    for (int i=0;i<this->topLevelItemCount();i++)
    {
        QTreeWidgetItem *item = this->topLevelItem(i);

        uint modelID = item->data(DOCUMENT_TREE_COLUMN_MODEL_ID,Qt::UserRole).toUInt();

        Model &rModel = Application::instance()->getSession()->getModel(modelID);
        QList<QString> documentFiles = rModel.getDocumentFiles();
        QVector<bool> documentExistenceBook;
        documentExistenceBook.fill(false,documentFiles.size());

        int j = item->childCount() - 1;
        while (j >= 0)
        {
            QTreeWidgetItem *childItem = item->child(j);

            int documentFilePosition = documentFiles.indexOf(childItem->text(DOCUMENT_TREE_COLUMN_FILENAME));
            if (documentFilePosition >= 0)
            {
                documentExistenceBook[documentFilePosition] = true;
            }
            else
            {
                item->takeChild(j);
            }
            j --;
        }

        for (j=0;j<documentFiles.size();j++)
        {
            if (!documentExistenceBook[j])
            {
                this->addDocument(item,modelID,documentFiles[j]);
            }
        }
    }

    this->blockSignals(false);

    Application::instance()->getSession()->unlock();
}

void DocumentTree::onModelAdded(uint modelID)
{
    R_LOG_TRACE;
    this->blockSignals(true);
    Model &rModel = Application::instance()->getSession()->getModel(modelID);

    QList<QString> documentFiles = rModel.getDocumentFiles();

    QTreeWidgetItem *item = new QTreeWidgetItem(this);
    item->setText(DOCUMENT_TREE_COLUMN_NAME,rModel.getName());
    item->setData(DOCUMENT_TREE_COLUMN_MODEL_ID,Qt::UserRole,QVariant(modelID));
    item->setData(DOCUMENT_TREE_COLUMN_IS_DOCUMENT,Qt::UserRole,QVariant(false));

    for (int i=0;i<documentFiles.size();i++)
    {
        this->addDocument(item,modelID,documentFiles[i]);
    }
    this->blockSignals(false);
}

void DocumentTree::onModelChanged(uint modelID)
{
    R_LOG_TRACE;
    for (int i=0;i<this->topLevelItemCount();i++)
    {
        QTreeWidgetItem *item = this->topLevelItem(i);
        if (item->data(DOCUMENT_TREE_COLUMN_MODEL_ID,Qt::UserRole).toUInt() == modelID)
        {
            Model &rModel = Application::instance()->getSession()->getModel(modelID);
            item->setText(DOCUMENT_TREE_COLUMN_NAME,rModel.getName());
        }
    }
}

void DocumentTree::onModelRemoved(uint modelID)
{
    R_LOG_TRACE;
    int i = this->topLevelItemCount() - 1;
    while (i >= 0)
    {
        if (this->topLevelItem(i)->data(DOCUMENT_TREE_COLUMN_MODEL_ID,Qt::UserRole).toUInt() == modelID)
        {
            delete this->takeTopLevelItem(i);
        }
        i--;
    }
    for (i=0;i<this->topLevelItemCount();i++)
    {
        QTreeWidgetItem *item = this->topLevelItem(i);
        uint oldModelID = item->data(DOCUMENT_TREE_COLUMN_MODEL_ID,Qt::UserRole).toUInt();
        if (oldModelID > modelID)
        {
            item->setData(DOCUMENT_TREE_COLUMN_MODEL_ID,Qt::UserRole,QVariant(oldModelID-1));
        }
    }
}

void DocumentTree::onItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    R_LOG_TRACE;
    if (column == DOCUMENT_TREE_COLUMN_NAME)
    {
        if (item->data(DOCUMENT_TREE_COLUMN_IS_DOCUMENT,Qt::UserRole).toBool())
        {
            QString fileName = item->text(DOCUMENT_TREE_COLUMN_FILENAME);

            RLogger::info("Opening the file \'%s\'\n",fileName.toUtf8().constData());

            if (!QDesktopServices::openUrl(QUrl("file:///" + fileName)))
            {
                RLogger::error("Failed to open the file \'%s\'\n",fileName.toUtf8().constData());
            }
        }
    }
}
