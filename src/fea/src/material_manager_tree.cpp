#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QSignalBlocker>
#include <QSet>

#include "material_library_loader.h"

#include <rbl_logger.h>

#include <rgl_message_box.h>

#include "material_manager_tree.h"
#include "application.h"
#include "session.h"
#include "application_settings.h"
#include "session_entity_id.h"
#include <rgl_icon.h>
#include <rml_entity_group.h>

MaterialManagerTree::MaterialManagerTree(QWidget *parent)
    : QWidget{parent}
    , problemTypeMask{R_PROBLEM_NONE}
{
    R_LOG_TRACE_IN;

    QIcon newIcon = RIcon::fromFile(":/icons/file/pixmaps/range-material_new.svg");
    QIcon deleteIcon = RIcon::fromFile(":/icons/file/pixmaps/range-material_delete.svg");

    QVBoxLayout *mainLayout = new QVBoxLayout;
    this->setLayout(mainLayout);

    this->treeWidget = new QTreeWidget;
    mainLayout->addWidget(this->treeWidget);
    this->loadingLabel = new QLabel(tr("Loading materials…"),this);
    mainLayout->addWidget(this->loadingLabel);
    this->treeWidget->setEnabled(false);

    this->treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    this->treeWidget->setSortingEnabled(true);
    this->treeWidget->sortByColumn(ColumnName,Qt::AscendingOrder);

    this->treeWidget->setColumnCount(ColumnType::NunberOfColumns);

    QTreeWidgetItem* headerItem = new QTreeWidgetItem();
    headerItem->setText(ColumnName,tr("Name"));
    headerItem->setText(ColumnFile,tr("File"));
    this->treeWidget->setHeaderItem(headerItem);
    this->treeWidget->setRootIsDecorated(false);
    this->treeWidget->resizeColumnToContents(ColumnName);
    this->treeWidget->resizeColumnToContents(ColumnFile);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    mainLayout->addLayout(buttonLayout);

    this->newButton = new QPushButton(newIcon,tr("New"));
    buttonLayout->addWidget(this->newButton);
    this->newButton->setEnabled(false);

    this->deleteButton = new QPushButton(deleteIcon,tr("Delete"));
    this->deleteButton->setDisabled(true);
    buttonLayout->addWidget(this->deleteButton);

    QObject::connect(this->treeWidget, &QTreeWidget::itemChanged, this, &MaterialManagerTree::onItemChanged);
    QObject::connect(this->treeWidget, &QTreeWidget::itemDoubleClicked, this, &MaterialManagerTree::onItemDoubleClicked);
    QObject::connect(this->treeWidget, &QTreeWidget::itemSelectionChanged, this, &MaterialManagerTree::onItemSelectionChanged);

    QObject::connect(this->newButton, &QPushButton::clicked, this, &MaterialManagerTree::onMaterialNew);
    QObject::connect(this->deleteButton, &QPushButton::clicked, this, &MaterialManagerTree::onMaterialDelete);

    QObject::connect(Application::instance()->getSession(), &Session::modelSelectionChanged, this, &MaterialManagerTree::onModelSelectionChanged);
    QObject::connect(Application::instance()->getSession(), &Session::modelAdded, this, &MaterialManagerTree::onModelAdded,Qt::BlockingQueuedConnection);
    QObject::connect(Application::instance()->getSession(), &Session::problemSelectionChanged, this, &MaterialManagerTree::onProblemChanged);

    // Every refresh reads all material files, so a burst of changes (e.g. the
    // material database being copied in) is refreshed only once it settles.
    this->directoryChangedTimer = new QTimer(this);
    this->directoryChangedTimer->setSingleShot(true);
    this->directoryChangedTimer->setInterval(500);
    QObject::connect(this->directoryChangedTimer,&QTimer::timeout,this,[this](){
        this->onDirectoryChanged(Application::instance()->getApplicationSettings()->getMaterialsDir());
    });

    QFileSystemWatcher *fileSystemWatcher = new QFileSystemWatcher(this);
    fileSystemWatcher->addPath(Application::instance()->getApplicationSettings()->getMaterialsDir());
    QObject::connect(fileSystemWatcher,&QFileSystemWatcher::directoryChanged,this->directoryChangedTimer,qOverload<>(&QTimer::start));

    // Start only after construction, once the editor connections exist and the
    // main window can be displayed.
    QTimer::singleShot(0,this,[this](){
        this->onDirectoryChanged(Application::instance()->getApplicationSettings()->getMaterialsDir());
    });
    QObject::connect(Application::instance()->getSession(),&Session::modelRemoved,this,[this](uint removed){
        QList<uint> remaining;
        for (uint modelID : std::as_const(this->pendingModelIDs))
        {
            if (modelID != removed)
            {
                remaining.append(modelID > removed ? modelID - 1 : modelID);
            }
        }
        this->pendingModelIDs = remaining;
    });

    R_LOG_TRACE_OUT;
}

MaterialManagerTree::~MaterialManagerTree()
{
    // Explicitly join before QObject destroys the worker and GUI children.
    delete this->materialLoader;
}

void MaterialManagerTree::updateCheckStates()
{
    R_LOG_TRACE_IN;
    QList<SessionEntityID> selectedEntityIDs = Application::instance()->getSession()->getSelectedEntityIDs();
    QMap<QString,bool> selectedMaterialNames;

    for (int i=0;i<selectedEntityIDs.size();i++)
    {
        QString materialName;
        switch (selectedEntityIDs[i].getType())
        {
            case R_ENTITY_GROUP_POINT:
                materialName = Application::instance()->getSession()->getModel(selectedEntityIDs[i].getMid()).getPoint(selectedEntityIDs[i].getEid()).getMaterial().getName();
                break;
            case R_ENTITY_GROUP_LINE:
                materialName = Application::instance()->getSession()->getModel(selectedEntityIDs[i].getMid()).getLine(selectedEntityIDs[i].getEid()).getMaterial().getName();
                break;
            case R_ENTITY_GROUP_SURFACE:
                materialName = Application::instance()->getSession()->getModel(selectedEntityIDs[i].getMid()).getSurface(selectedEntityIDs[i].getEid()).getMaterial().getName();
                break;
            case R_ENTITY_GROUP_VOLUME:
                materialName = Application::instance()->getSession()->getModel(selectedEntityIDs[i].getMid()).getVolume(selectedEntityIDs[i].getEid()).getMaterial().getName();
                break;
            default:
                break;
        }
        selectedMaterialNames[materialName] = true;
    }

    Qt::CheckState state;
    switch (selectedMaterialNames.size())
    {
        case 0:
            state = Qt::Unchecked;
            break;
        case 1:
            state = Qt::Checked;
            break;
        default:
            state = Qt::PartiallyChecked;
            break;
    }

    bool signalsBlockedOld = this->treeWidget->signalsBlocked();
    this->treeWidget->blockSignals(true);

    for (int i=0;i<this->treeWidget->topLevelItemCount();i++)
    {
        if (selectedMaterialNames.find(this->treeWidget->topLevelItem(i)->text(ColumnType::ColumnName)) == selectedMaterialNames.end())
        {
            this->treeWidget->topLevelItem(i)->setData(ColumnType::ColumnName,Qt::CheckStateRole,QVariant(Qt::Unchecked));
            this->treeWidget->topLevelItem(i)->setData(ColumnType::ColumnFile,Qt::UserRole,QVariant(Qt::Unchecked));
        }
        else
        {
            this->treeWidget->topLevelItem(i)->setData(ColumnType::ColumnName,Qt::CheckStateRole,QVariant(state));
            this->treeWidget->topLevelItem(i)->setData(ColumnType::ColumnFile,Qt::UserRole,QVariant(state));
        }
    }

    this->treeWidget->blockSignals(signalsBlockedOld);
    R_LOG_TRACE_OUT;
}

void MaterialManagerTree::updateProblemTypeMask()
{
    R_LOG_TRACE_IN;
    QList<uint> selectedModelIDs = Application::instance()->getSession()->getSelectedModelIDs();

    this->problemTypeMask = R_PROBLEM_NONE;
    for (int i=0;i<selectedModelIDs.size();i++)
    {
        this->problemTypeMask |= Application::instance()->getSession()->getModel(selectedModelIDs[i]).getProblemTaskTree().getProblemTypeMask();
    }

    bool signalsBlockedOld = this->treeWidget->signalsBlocked();
    this->treeWidget->blockSignals(true);
    for (int i=0;i<this->treeWidget->topLevelItemCount();i++)
    {
        QTreeWidgetItem *item = this->treeWidget->topLevelItem(i);
        this->setItemValid(item,this->findItemMaterial(item).validForProblemType(this->problemTypeMask));
    }
    this->treeWidget->blockSignals(signalsBlockedOld);
    R_LOG_TRACE_OUT
}

void MaterialManagerTree::setItemValid(QTreeWidgetItem *item, bool valid)
{
    R_LOG_TRACE;
    QFont font = item->font(ColumnType::ColumnName);
    font.setBold(valid);
    font.setItalic(!valid);
    item->setFont(ColumnType::ColumnName,font);
}

void MaterialManagerTree::updateItem(QTreeWidgetItem *item, const RMaterial &material, const QString &filePath, bool setSelected)
{
    R_LOG_TRACE_IN;

    this->materialCache.insert(filePath,material);

    bool signalsBlockedOld = this->treeWidget->signalsBlocked();
    this->treeWidget->blockSignals(true);
    item->setText(ColumnType::ColumnName,material.getName());
    item->setData(ColumnType::ColumnName,Qt::UserRole,QVariant(material.getName()));
    item->setData(ColumnType::ColumnName,Qt::CheckStateRole,QVariant(Qt::Unchecked));
    this->setItemValid(item,material.validForProblemType(this->problemTypeMask));
    item->setText(ColumnType::ColumnFile,filePath);
    // item->setData(ColumnType::ColumnFile,Qt::UserRole,QVariant(material.getID()));
    item->setData(ColumnType::ColumnFile,Qt::UserRole,QVariant(Qt::Unchecked));
    QFont font = item->font(ColumnType::ColumnFile);
    font.setItalic(true);
    item->setFont(ColumnType::ColumnFile,font);
    this->treeWidget->resizeColumnToContents(ColumnType::ColumnName);
    this->treeWidget->resizeColumnToContents(ColumnType::ColumnFile);
    this->treeWidget->blockSignals(signalsBlockedOld);
    if (setSelected)
    {
        this->treeWidget->clearSelection();
        item->setSelected(true);
    }

    R_LOG_TRACE_OUT;
}

RMaterial MaterialManagerTree::findItemMaterial(QTreeWidgetItem *item, bool *ok)
{
    R_LOG_TRACE_IN;

    if (ok)
    {
        *ok = true;
    }

    QString filePath = item->text(ColumnType::ColumnFile);

    QMap<QString,RMaterial>::const_iterator iter = this->materialCache.constFind(filePath);
    if (iter != this->materialCache.constEnd())
    {
        R_LOG_TRACE_RETURN(iter.value());
    }

    if (ok)
    {
        *ok = false;
    }
    R_LOG_TRACE_RETURN(RMaterial());
}

QStringList MaterialManagerTree::findFiles(const QString &name) const
{
    R_LOG_TRACE_IN;
    QStringList fileNames;
    for (int i=0; i<this->treeWidget->topLevelItemCount(); i++)
    {
        if (name == this->treeWidget->topLevelItem(i)->text(ColumnType::ColumnName))
        {
            fileNames.append(this->treeWidget->topLevelItem(i)->text(ColumnType::ColumnFile));
        }
    }
    R_LOG_TRACE_RETURN(fileNames);
}

QString MaterialManagerTree::findAvailableName(const QString &name) const
{
    R_LOG_TRACE_IN;
    QString newName(name);

    uint incr = 0;
    bool findNext = true;
    while (findNext)
    {
        findNext = false;
        for (int i=0; i<this->treeWidget->topLevelItemCount(); i++)
        {
            if (newName == this->treeWidget->topLevelItem(i)->text(ColumnType::ColumnName))
            {
                newName = name + " - " + QString::number(++incr);
                findNext = true;
                break;
            }
        }
    }
    R_LOG_TRACE_RETURN(newName);
}

int MaterialManagerTree::compareModelMaterial(const RMaterial &material) const
{
    R_LOG_TRACE_IN;

    QStringList fileNames = this->findFiles(material.getName());
    if (fileNames.empty())
    {
        R_LOG_TRACE_RETURN(-1);
    }

    for (const QString &filePath : std::as_const(fileNames))
    {
        const auto materialIter = this->materialCache.constFind(filePath);
        if (materialIter != this->materialCache.constEnd() && materialIter->propertiesEqual(material))
        {
            R_LOG_TRACE_RETURN(0);
        }
    }
    R_LOG_TRACE_RETURN(1);
}

QString MaterialManagerTree::buildFilePath(const RMaterial &material)
{
    QDir materialsDir(Application::instance()->getApplicationSettings()->getMaterialsDir());
    return materialsDir.filePath(material.getID().toString(QUuid::WithoutBraces) + "." + RMaterial::getDefaultFileExtension());
}

void MaterialManagerTree::write(const QString &fileName, const RMaterial &material)
{
    material.write(fileName);
}

void MaterialManagerTree::onMaterialChanged(const RMaterial &material)
{
    QString filePath = MaterialManagerTree::buildFilePath(material);
    if (this->materialLoader)
    {
        // An edit delivered while a snapshot is being read must not be replaced
        // by that older snapshot.
        this->refreshPending = true;
    }
    // Keep the cache in step with the file right away rather than waiting for
    // the directory watcher to report the write.
    this->materialCache.insert(filePath,material);
    try
    {
        MaterialManagerTree::write(filePath,material);
    }
    catch (const RError &error)
    {
        RLogger::error("Failed to write material file \"%s\". %s\n", filePath.toUtf8().constData(), error.getMessage().toUtf8().constData());
    }
}

void MaterialManagerTree::onDirectoryChanged(const QString &path)
{
    this->directoryChangedTimer->stop();
    if (this->materialLoader)
    {
        this->refreshPending = true;
        return;
    }

    this->materialsReady = false;
    this->treeWidget->setEnabled(false);
    this->newButton->setEnabled(false);
    this->deleteButton->setEnabled(false);
    this->loadingLabel->setText(tr("Loading materials…"));
    this->loadingLabel->show();
    emit this->loadingChanged(true);

    this->materialLoader = new MaterialLibraryLoader(path,this);
    QObject::connect(this->materialLoader,&MaterialLibraryLoader::progressChanged,this,[this](int completed,int total){
        this->loadingLabel->setText(tr("Loading materials… %1/%2").arg(completed).arg(total));
    });
    QObject::connect(this->materialLoader,&QThread::finished,this,&MaterialManagerTree::onMaterialsLoaded);
    this->materialLoader->start();
}

void MaterialManagerTree::onMaterialsLoaded()
{
    MaterialLibraryLoader *loader = this->materialLoader;
    this->materialLoader = nullptr;
    loader->deleteLater();

    for (const QString &error : std::as_const(loader->errors))
    {
        RLogger::error("%s\n",error.toUtf8().constData());
    }
    if (this->refreshPending || this->directoryChangedTimer->isActive())
    {
        // Serialize refreshes and discard snapshots invalidated while reading.
        this->refreshPending = false;
        this->onDirectoryChanged(Application::instance()->getApplicationSettings()->getMaterialsDir());
        return;
    }

    const QSignalBlocker blocker(this->treeWidget);
    this->treeWidget->setSortingEnabled(false);
    this->materialCache = std::move(loader->materials);
    this->updateProblemTypeMask();
    for (int i=this->treeWidget->topLevelItemCount()-1; i>=0; --i)
    {
        QTreeWidgetItem *item = this->treeWidget->topLevelItem(i);
        const auto iter = this->materialCache.constFind(item->text(ColumnType::ColumnFile));
        if (iter == this->materialCache.constEnd())
        {
            delete this->treeWidget->takeTopLevelItem(i);
        }
        else
        {
            const RMaterial material(iter.value());
            const QString filePath(iter.key());
            this->updateItem(item,material,filePath,false);
        }
    }
    QSet<QString> existingPaths;
    for (int i=0; i<this->treeWidget->topLevelItemCount(); ++i)
    {
        existingPaths.insert(this->treeWidget->topLevelItem(i)->text(ColumnType::ColumnFile));
    }
    for (auto iter = this->materialCache.cbegin(); iter != this->materialCache.cend(); ++iter)
    {
        if (!existingPaths.contains(iter.key()))
        {
            const RMaterial material(iter.value());
            const QString filePath(iter.key());
            this->updateItem(new QTreeWidgetItem(this->treeWidget),material,filePath,false);
        }
    }
    this->treeWidget->setSortingEnabled(true);
    this->updateCheckStates();
    // Failed reads do not establish that a model material is absent. Keep model
    // reconciliation queued until a complete snapshot can be loaded.
    this->materialsReady = loader->errors.isEmpty();
    this->treeWidget->setEnabled(true);
    this->newButton->setEnabled(true);
    this->loadingLabel->setVisible(!loader->errors.isEmpty());
    this->loadingLabel->setText(tr("Some materials could not be loaded. See the application log."));
    emit this->loadingChanged(false);
    this->onItemSelectionChanged();

    // A model can arrive before the initial snapshot. Compare it only against
    // the complete library, never against a partially loaded tree.
    QTimer::singleShot(0,this,&MaterialManagerTree::processPendingModels);
}

void MaterialManagerTree::processPendingModels()
{
    if (!this->materialsReady || this->pendingModelIDs.isEmpty())
    {
        return;
    }
    const uint modelID = this->pendingModelIDs.takeFirst();
    if (modelID < Application::instance()->getSession()->getNModels())
    {
        this->onModelAdded(modelID);
    }
    QTimer::singleShot(0,this,&MaterialManagerTree::processPendingModels);
}

void MaterialManagerTree::onModelSelectionChanged(uint)
{
    R_LOG_TRACE_IN;
    this->updateProblemTypeMask();
    this->updateCheckStates();
    R_LOG_TRACE_OUT;
}

void MaterialManagerTree::onModelAdded(uint modelID)
{
    R_LOG_TRACE_IN;
    if (!this->materialsReady)
    {
        if (!this->pendingModelIDs.contains(modelID))
        {
            this->pendingModelIDs.append(modelID);
        }
        R_LOG_TRACE_OUT;
        return;
    }
    QMap<QString,RMaterial> unknownMaterials;
    QMap<QString,RMaterial> differentMaterials;

    // Loop over all element groups and check if it contains unknown material.
    QList<RMaterial> materialList;

    for (uint j=0;j<Application::instance()->getSession()->getModel(modelID).getNPoints();j++)
    {
        const RMaterial &material = Application::instance()->getSession()->getModel(modelID).getPoint(j).getMaterial();
        if (!material.getName().isEmpty())
        {
            materialList.append(material);
        }
    }
    for (uint j=0;j<Application::instance()->getSession()->getModel(modelID).getNLines();j++)
    {
        const RMaterial &material = Application::instance()->getSession()->getModel(modelID).getLine(j).getMaterial();
        if (!material.getName().isEmpty())
        {
            materialList.append(material);
        }
    }
    for (uint j=0;j<Application::instance()->getSession()->getModel(modelID).getNSurfaces();j++)
    {
        const RMaterial &material = Application::instance()->getSession()->getModel(modelID).getSurface(j).getMaterial();
        if (!material.getName().isEmpty())
        {
            materialList.append(material);
        }
    }
    for (uint j=0;j<Application::instance()->getSession()->getModel(modelID).getNVolumes();j++)
    {
        const RMaterial &material = Application::instance()->getSession()->getModel(modelID).getVolume(j).getMaterial();
        if (!material.getName().isEmpty())
        {
            materialList.append(material);
        }
    }

    for (const RMaterial &material : materialList)
    {
        int retVal = this->compareModelMaterial(material);
        if (retVal < 0)
        {
            unknownMaterials[material.getName()] = material;
        }
        else if (retVal > 0)
        {
            differentMaterials[material.getName()] = material;
        }
    }

    bool addUnknown = false;
    if (unknownMaterials.size() > 0)
    {
        QString materialNames;
        QMap<QString,RMaterial>::const_iterator iter;
        for (iter = unknownMaterials.constBegin(); iter != unknownMaterials.constEnd(); ++iter)
        {
            materialNames += "<li><b>" + iter.key() + "</b></li>";
        }

        QString question = "Following materials are not present in your database:<ol>"
                         + materialNames
                         + "</ol>Do you want to add them to the material list?";
        addUnknown = RMessageBox::question(this,"Unknown materials",question) == RMessageBox::Yes;
    }

    if (addUnknown)
    {
        QMap<QString,RMaterial>::iterator iter;
        for (iter = unknownMaterials.begin(); iter != unknownMaterials.end(); ++iter)
        {
            RMaterial &material = iter.value();
            material.setName(MaterialManagerTree::findAvailableName(iter.key()));
            QString filePath = MaterialManagerTree::buildFilePath(material);

            try
            {
                MaterialManagerTree::write(filePath,material);
            }
            catch (const RError &error)
            {
                RLogger::error("Failed to write material file \"%s\". %s\n", filePath.toUtf8().constData(), error.getMessage().toUtf8().constData());
            }
        }
    }
    else
    {
        QMap<QString,RMaterial>::const_iterator iter;
        for (iter = unknownMaterials.constBegin(); iter != unknownMaterials.constEnd(); ++iter)
        {
            // Remove unknown material from the model's element groups.
            for (uint j=0;j<Application::instance()->getSession()->getModel(modelID).getNPoints();j++)
            {
                if (Application::instance()->getSession()->getModel(modelID).getPoint(j).getMaterial().getName() == iter.key())
                {
                    Application::instance()->getSession()->getModel(modelID).getPoint(j).setMaterial(RMaterial());
                }
            }
            for (uint j=0;j<Application::instance()->getSession()->getModel(modelID).getNLines();j++)
            {
                if (Application::instance()->getSession()->getModel(modelID).getLine(j).getMaterial().getName() == iter.key())
                {
                    Application::instance()->getSession()->getModel(modelID).getLine(j).setMaterial(RMaterial());
                }
            }
            for (uint j=0;j<Application::instance()->getSession()->getModel(modelID).getNSurfaces();j++)
            {
                if (Application::instance()->getSession()->getModel(modelID).getSurface(j).getMaterial().getName() == iter.key())
                {
                    Application::instance()->getSession()->getModel(modelID).getSurface(j).setMaterial(RMaterial());
                }
            }
            for (uint j=0;j<Application::instance()->getSession()->getModel(modelID).getNVolumes();j++)
            {
                if (Application::instance()->getSession()->getModel(modelID).getVolume(j).getMaterial().getName() == iter.key())
                {
                    Application::instance()->getSession()->getModel(modelID).getVolume(j).setMaterial(RMaterial());
                }
            }
        }
    }

    bool addDifferent = false;
    if (differentMaterials.size() > 0)
    {
        QString materialNames;
        QMap<QString,RMaterial>::const_iterator iter;
        for (iter = differentMaterials.constBegin(); iter != differentMaterials.constEnd(); ++iter)
        {
            materialNames += "<li><b>" + iter.key() + "</b></li>";
        }

        QString question = "Model contains following materials which already exist in your database and have different properties:<ol>"
                         + materialNames
                         + "</ol>Do you want to add them to the material list with different names?<br/>If not material from database will be applied.";
        addDifferent = RMessageBox::question(this,"Different materials",question) == RMessageBox::Yes;
    }

    if (addDifferent)
    {
        QMap<QString,RMaterial>::iterator iter;
        for (iter = differentMaterials.begin(); iter != differentMaterials.end(); ++iter)
        {
            RMaterial &material = iter.value();
            material.setName(MaterialManagerTree::findAvailableName(iter.key() + " - " + tr("Imported")));
            QString filePath = MaterialManagerTree::buildFilePath(material);

            try
            {
                MaterialManagerTree::write(filePath,material);
            }
            catch (const RError &error)
            {
                RLogger::error("Failed to write material file \"%s\". %s\n", filePath.toUtf8().constData(), error.getMessage().toUtf8().constData());
            }
        }

        QMap<QString,RMaterial>::const_iterator citer;
        for (citer = differentMaterials.constBegin(); citer != differentMaterials.constEnd(); ++citer)
        {
            // Remove unknown material from the model's element groups.
            for (uint j=0;j<Application::instance()->getSession()->getModel(modelID).getNPoints();j++)
            {
                if (Application::instance()->getSession()->getModel(modelID).getPoint(j).getMaterial().getName() == citer.key())
                {
                    Application::instance()->getSession()->getModel(modelID).getPoint(j).setMaterial(citer.value());
                }
            }
            for (uint j=0;j<Application::instance()->getSession()->getModel(modelID).getNLines();j++)
            {
                if (Application::instance()->getSession()->getModel(modelID).getLine(j).getMaterial().getName() == citer.key())
                {
                    Application::instance()->getSession()->getModel(modelID).getLine(j).setMaterial(citer.value());
                }
            }
            for (uint j=0;j<Application::instance()->getSession()->getModel(modelID).getNSurfaces();j++)
            {
                if (Application::instance()->getSession()->getModel(modelID).getSurface(j).getMaterial().getName() == citer.key())
                {
                    Application::instance()->getSession()->getModel(modelID).getSurface(j).setMaterial(citer.value());
                }
            }
            for (uint j=0;j<Application::instance()->getSession()->getModel(modelID).getNVolumes();j++)
            {
                if (Application::instance()->getSession()->getModel(modelID).getVolume(j).getMaterial().getName() == citer.key())
                {
                    Application::instance()->getSession()->getModel(modelID).getVolume(j).setMaterial(citer.value());
                }
            }
        }
    }
    else
    {
        QMap<QString,RMaterial>::const_iterator iter;
        for (iter = differentMaterials.constBegin(); iter != differentMaterials.constEnd(); ++iter)
        {
            // Remove unknown material from the model's element groups.
            for (uint j=0;j<Application::instance()->getSession()->getModel(modelID).getNPoints();j++)
            {
                if (Application::instance()->getSession()->getModel(modelID).getPoint(j).getMaterial().getName() == iter.key())
                {
                    Application::instance()->getSession()->getModel(modelID).getPoint(j).setMaterial(RMaterial());
                }
            }
            for (uint j=0;j<Application::instance()->getSession()->getModel(modelID).getNLines();j++)
            {
                if (Application::instance()->getSession()->getModel(modelID).getLine(j).getMaterial().getName() == iter.key())
                {
                    Application::instance()->getSession()->getModel(modelID).getLine(j).setMaterial(RMaterial());
                }
            }
            for (uint j=0;j<Application::instance()->getSession()->getModel(modelID).getNSurfaces();j++)
            {
                if (Application::instance()->getSession()->getModel(modelID).getSurface(j).getMaterial().getName() == iter.key())
                {
                    Application::instance()->getSession()->getModel(modelID).getSurface(j).setMaterial(RMaterial());
                }
            }
            for (uint j=0;j<Application::instance()->getSession()->getModel(modelID).getNVolumes();j++)
            {
                if (Application::instance()->getSession()->getModel(modelID).getVolume(j).getMaterial().getName() == iter.key())
                {
                    Application::instance()->getSession()->getModel(modelID).getVolume(j).setMaterial(RMaterial());
                }
            }
        }
    }
    R_LOG_TRACE_OUT;
}

void MaterialManagerTree::onProblemChanged(uint)
{
    R_LOG_TRACE_IN;
    this->updateProblemTypeMask();
    R_LOG_TRACE_OUT;
}

void MaterialManagerTree::onItemChanged(QTreeWidgetItem *item, int column)
{
    R_LOG_TRACE_IN;

    if (column != ColumnType::ColumnName)
    {
        R_LOG_TRACE_OUT;
        return;
    }

    QString oldName = item->data(ColumnType::ColumnName,Qt::UserRole).toString();
    QString newName = item->text(ColumnType::ColumnName);

    bool materialRead = false;
    RMaterial material = this->findItemMaterial(item,&materialRead);
    if (!materialRead)
    {
        R_LOG_TRACE_OUT;
        return;
    }

    // findItemMaterial() may have corrected the path while converting the file.
    QString filePath = item->text(ColumnType::ColumnFile);

    if (oldName != newName)
    {
        material.setName(newName);
        item->setData(ColumnType::ColumnName,Qt::UserRole,QVariant(newName));
        this->materialCache.insert(filePath,material);

        try
        {
            MaterialManagerTree::write(filePath,material);
        }
        catch (const RError &error)
        {
            RLogger::error("Failed to write material file \"%s\". %s\n", filePath.toUtf8().constData(), error.getMessage().toUtf8().constData());
        }

        emit this->materialSelected(material);
    }

    if (item->checkState(ColumnType::ColumnName) != Qt::PartiallyChecked &&
        item->checkState(ColumnType::ColumnName) != Qt::CheckState(item->data(ColumnType::ColumnFile,Qt::UserRole).toInt()))
    {
        if (item->checkState(ColumnType::ColumnName) != Qt::Checked)
        {
            material = RMaterial();
        }

        QList<SessionEntityID> selectedEntityIDs = Application::instance()->getSession()->getSelectedEntityIDs();

        for (int i=0;i<selectedEntityIDs.size();i++)
        {
            switch (selectedEntityIDs[i].getType())
            {
                case R_ENTITY_GROUP_POINT:
                    Application::instance()->getSession()->getModel(selectedEntityIDs[i].getMid()).getPoint(selectedEntityIDs[i].getEid()).setMaterial(material);
                    break;
                case R_ENTITY_GROUP_LINE:
                    Application::instance()->getSession()->getModel(selectedEntityIDs[i].getMid()).getLine(selectedEntityIDs[i].getEid()).setMaterial(material);
                    break;
                case R_ENTITY_GROUP_SURFACE:
                    Application::instance()->getSession()->getModel(selectedEntityIDs[i].getMid()).getSurface(selectedEntityIDs[i].getEid()).setMaterial(material);
                    break;
                case R_ENTITY_GROUP_VOLUME:
                    Application::instance()->getSession()->getModel(selectedEntityIDs[i].getMid()).getVolume(selectedEntityIDs[i].getEid()).setMaterial(material);
                    break;
                default:
                    break;
            }
        }
    }
    this->updateCheckStates();

    R_LOG_TRACE_OUT;
}

void MaterialManagerTree::onItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    R_LOG_TRACE_IN;
    this->blockSignals(true);
    if (column == ColumnType::ColumnName)
    {
        item->setFlags(item->flags() | Qt::ItemIsEditable);
    }
    else
    {
        if (item->flags() & Qt::ItemIsEditable)
        {
            item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        }
    }
    this->blockSignals(false);
    R_LOG_TRACE_OUT;
}

void MaterialManagerTree::onItemSelectionChanged()
{
    R_LOG_TRACE_IN;

    QList<QTreeWidgetItem*> items = this->treeWidget->selectedItems();

    this->deleteButton->setEnabled(items.count());

    if (items.count() == 0)
    {
        emit this->materialSelected(RMaterial());
    }
    else
    {
        for (QTreeWidgetItem *item : std::as_const(items))
        {
            emit this->materialSelected(this->findItemMaterial(item));
        }
    }
    R_LOG_TRACE_OUT;
}

void MaterialManagerTree::onMaterialNew()
{
    R_LOG_TRACE_IN;
    RMaterial material = RMaterial::generateDefault();
    material.setName(MaterialManagerTree::findAvailableName(tr("New material")));
    QString filePath = MaterialManagerTree::buildFilePath(material);

    try
    {
        MaterialManagerTree::write(filePath,material);
    }
    catch (const RError &error)
    {
        RLogger::error("Failed to write material file \"%s\". %s\n", filePath.toUtf8().constData(), error.getMessage().toUtf8().constData());
    }
    R_LOG_TRACE_OUT;
}

void MaterialManagerTree::onMaterialDelete()
{
    R_LOG_TRACE_IN;
    QList<QTreeWidgetItem*> selectedItems = this->treeWidget->selectedItems();
    for (QTreeWidgetItem *item : std::as_const(selectedItems))
    {
        QString fileName = item->text(ColumnType::ColumnName);
        QString filePath = item->text(ColumnType::ColumnFile);

        QString question = tr("Are you sure you want to delete selected material?")
                           + "<p>" + fileName + "<pre>" + filePath + "</pre></p>";

        if (RMessageBox::question(this,tr("Remove material"),question) == RMessageBox::Yes)
        {
            if (!QFile::remove(filePath))
            {
                RLogger::error("Failed to delete material file \"%s\".\n", filePath.toUtf8().constData());
            }
        }
    }
    R_LOG_TRACE_OUT;
}
