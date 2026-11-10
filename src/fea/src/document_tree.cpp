#include <QDesktopServices>
#include <QFileInfo>
#include <QTimer>

#include "document_tree.h"
#include "application.h"
#include "application_settings.h"
#include "session.h"
#include "model.h"
#include <rgl_icon.h>
#include <rbl_logger.h>

typedef enum _DocumentTreeColumn
{
    DOCUMENT_TREE_COLUMN_NAME = 0,
    DOCUMENT_TREE_COLUMN_FILENAME,
    DOCUMENT_TREE_COLUMN_MODEL_ID,
    DOCUMENT_TREE_COLUMN_IS_DOCUMENT,
    DOCUMENT_TREE_N_COLUMNS
} DocumentTreeColumn;

typedef enum _DocumentTreeCategory
{
    DOCUMENT_TREE_CATEGORY_DOCUMENTS = 0,
    DOCUMENT_TREE_CATEGORY_VIDEOS,
    DOCUMENT_TREE_CATEGORY_IMAGES
} DocumentTreeCategory;

//! Return category of given file.
static int findDocumentCategory(const QString &filename)
{
    const QString suffix(QFileInfo(filename).suffix());

    if (ApplicationSettings::getSupportedImageFormats().contains(suffix,Qt::CaseInsensitive))
    {
        return DOCUMENT_TREE_CATEGORY_IMAGES;
    }
    if (ApplicationSettings::getSupportedVideoFormats().contains(suffix,Qt::CaseInsensitive))
    {
        return DOCUMENT_TREE_CATEGORY_VIDEOS;
    }
    return DOCUMENT_TREE_CATEGORY_DOCUMENTS;
}

DocumentTree::DocumentTree(QWidget *parent) :
    ModelColorTree(parent)
{
    R_LOG_TRACE;
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);
    this->setRootIsDecorated(true);
    this->setAutoScroll(true);

    QTreeWidgetItem* headerItem = new QTreeWidgetItem();
    headerItem->setText(DOCUMENT_TREE_COLUMN_NAME,QString(tr("File name")));
    headerItem->setIcon(DOCUMENT_TREE_COLUMN_NAME,RIcon::fromFile(":/icons/mime/pixmaps/range-file-document.svg"));
    this->setHeaderItem(headerItem);

    // Header item defines number of columns, so this must follow it.
    this->setColumnCount(DOCUMENT_TREE_N_COLUMNS);

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

uint DocumentTree::findRowModelID(const QModelIndex &index) const
{
    return index.siblingAtColumn(DOCUMENT_TREE_COLUMN_MODEL_ID).data(Qt::UserRole).toUInt();
}

void DocumentTree::addDocument(QTreeWidgetItem *item, uint modelID, const QString &filename)
{
    R_LOG_TRACE;
    QFileInfo fi(filename);

    const int category = findDocumentCategory(filename);
    QTreeWidgetItem *categoryItem = this->findCategoryItem(item,modelID,category);

    QTreeWidgetItem *childItem = new QTreeWidgetItem;

    childItem->setText(DOCUMENT_TREE_COLUMN_NAME,fi.fileName());
    childItem->setText(DOCUMENT_TREE_COLUMN_FILENAME,filename);
    childItem->setData(DOCUMENT_TREE_COLUMN_MODEL_ID,Qt::UserRole,QVariant(modelID));
    childItem->setData(DOCUMENT_TREE_COLUMN_IS_DOCUMENT,Qt::UserRole,QVariant(true));

    // Files are usually added in order, so the search starts at the end.
    int position = categoryItem->childCount();
    while (position > 0 && fi.fileName().compare(categoryItem->child(position-1)->text(DOCUMENT_TREE_COLUMN_NAME),Qt::CaseInsensitive) < 0)
    {
        position--;
    }
    categoryItem->insertChild(position,childItem);

    if (categoryItem->childCount() == 1)
    {
        categoryItem->setExpanded(true);
    }
}

QTreeWidgetItem *DocumentTree::findCategoryItem(QTreeWidgetItem *item, uint modelID, int category)
{
    int position = 0;
    for (;position<item->childCount();position++)
    {
        const int childCategory = item->child(position)->data(DOCUMENT_TREE_COLUMN_NAME,Qt::UserRole).toInt();
        if (childCategory == category)
        {
            return item->child(position);
        }
        if (childCategory > category)
        {
            break;
        }
    }

    QTreeWidgetItem *categoryItem = new QTreeWidgetItem;

    switch (category)
    {
        case DOCUMENT_TREE_CATEGORY_IMAGES:
            categoryItem->setText(DOCUMENT_TREE_COLUMN_NAME,tr("Images"));
            break;
        case DOCUMENT_TREE_CATEGORY_VIDEOS:
            categoryItem->setText(DOCUMENT_TREE_COLUMN_NAME,tr("Videos"));
            break;
        default:
            categoryItem->setText(DOCUMENT_TREE_COLUMN_NAME,tr("Documents"));
            break;
    }
    categoryItem->setData(DOCUMENT_TREE_COLUMN_NAME,Qt::UserRole,QVariant(category));
    categoryItem->setData(DOCUMENT_TREE_COLUMN_MODEL_ID,Qt::UserRole,QVariant(modelID));
    categoryItem->setData(DOCUMENT_TREE_COLUMN_IS_DOCUMENT,Qt::UserRole,QVariant(false));

    item->insertChild(position,categoryItem);

    return categoryItem;
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

        for (int g=item->childCount()-1;g>=0;g--)
        {
            QTreeWidgetItem *categoryItem = item->child(g);

            for (int c=categoryItem->childCount()-1;c>=0;c--)
            {
                int documentFilePosition = documentFiles.indexOf(categoryItem->child(c)->text(DOCUMENT_TREE_COLUMN_FILENAME));
                if (documentFilePosition >= 0)
                {
                    documentExistenceBook[documentFilePosition] = true;
                }
                else
                {
                    delete categoryItem->takeChild(c);
                }
            }

            if (categoryItem->childCount() == 0)
            {
                delete item->takeChild(g);
            }
        }

        for (int j=0;j<documentFiles.size();j++)
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
    item->setExpanded(true);
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
