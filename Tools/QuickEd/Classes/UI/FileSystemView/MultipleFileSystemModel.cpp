#include "MultipleFileSystemModel.h"
#include "Debug/DVAssert.h"

#include <QFileSystemModel>
#include "FileSystemModel.h"

MultipleFileSystemModel::MultipleFileSystemModel(QObject* parent /*= nullptr*/)
    : fileSystemModels()
{

}

MultipleFileSystemModel::~MultipleFileSystemModel()
{
}

QModelIndex MultipleFileSystemModel::addPath(const QString& path, const QString& alias)
{
    QFileSystemModel* model = new FileSystemModel(this);
    QModelIndex root = model->setRootPath(path);

    FileSystemInfo info;
    info.path = path;
    info.alias = alias;
    info.model = model;
    info.root = root;

    fileSystemModels.push_back(info);

    ConnectToModel(model);

    int row = (fileSystemModels.size() - 1);
    return createIndex(row, 0, row);
}

void MultipleFileSystemModel::delPath(const QString& path)
{
    auto it = std::find_if(fileSystemModels.begin(), fileSystemModels.end(), [path](const FileSystemInfo& info)
                           {
                               return info.path == path;
                           });

    if (it != fileSystemModels.end())
    {
        beginResetModel();
        DisconnectFromModel(it->model);
        fileSystemModels.erase(it);
        mappingToSource.clear();
        mappingFromSource.clear();
        endResetModel();
    }
}

void MultipleFileSystemModel::delAllPaths()
{
    beginResetModel();
    for (FileSystemInfo& info : fileSystemModels)
    {
        DisconnectFromModel(info.model);
    }

    fileSystemModels.clear();
    mappingToSource.clear();
    mappingFromSource.clear();
    endResetModel();
}

void MultipleFileSystemModel::setFilter(QDir::Filters aFilters)
{
    filters = aFilters;
    for (auto& info : fileSystemModels)
    {
        info.model->setFilter(filters);
    }
}

QDir::Filters MultipleFileSystemModel::filter() const
{
    return filters;
}

void MultipleFileSystemModel::setReadOnly(bool enable)
{
    readOnlyEnable = enable;
    for (auto& info : fileSystemModels)
    {
        info.model->setReadOnly(readOnlyEnable);
    }
}

bool MultipleFileSystemModel::isReadOnly() const
{
    return readOnlyEnable;
}

void MultipleFileSystemModel::setNameFilterDisables(bool enable)
{
    nameFilterEnabled = enable;
    for (auto& info : fileSystemModels)
    {
        info.model->setNameFilterDisables(nameFilterEnabled);
    }
}

bool MultipleFileSystemModel::nameFilterDisables() const
{
    return nameFilterEnabled;
}

void MultipleFileSystemModel::setNameFilters(const QStringList& filters)
{
    localNameFilters = filters;
    for (auto& info : fileSystemModels)
    {
        info.model->setNameFilters(localNameFilters);
    }
}

QStringList MultipleFileSystemModel::nameFilters() const
{
    return localNameFilters;
}

QModelIndex MultipleFileSystemModel::mkdir(const QModelIndex& parent, const QString& name)
{
    if (!parent.isValid())
    {
        return QModelIndex();
    }
    if (IsProxyRoot(parent))
    {
        return QModelIndex();
    }

    auto sourceParent = MapToSource(parent);
    return MapFromSource(GetModel(sourceParent)->mkdir(sourceParent, name));
}

QString MultipleFileSystemModel::fileName(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return QString();
    }
    if (IsProxyRoot(index))
    {
        return QString();
    }

    auto sourceIndex = MapToSource(index);
    return GetModel(sourceIndex)->fileName(sourceIndex);
}

bool MultipleFileSystemModel::remove(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return false;
    }
    if (IsProxyRoot(index))
    {
        return false;
    }

    auto sourceIndex = MapToSource(index);
    return GetModel(sourceIndex)->remove(sourceIndex);
}

QString MultipleFileSystemModel::filePath(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return QString();
    }
    if (IsProxyRoot(index))
    {
        int modelIdx = index.internalId();
        return GetModel(modelIdx)->filePath(GetRoot(modelIdx));
    }

    auto sourceIndex = MapToSource(index);
    return GetModel(sourceIndex)->filePath(sourceIndex);
}

bool MultipleFileSystemModel::isDir(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return false;
    }
    if (IsProxyRoot(index))
    {
        return true;
    }

    auto sourceIndex = MapToSource(index);
    return GetModel(sourceIndex)->isDir(sourceIndex);
}

QModelIndex MultipleFileSystemModel::index(const QString& path, int column /*= 0*/) const
{
    for (auto& info : fileSystemModels)
    {
        QModelIndex index = info.model->index(path, column);
        if (index.isValid())
        {
            return MapFromSource(index);
        }
    }

    return QModelIndex();
}

QModelIndex MultipleFileSystemModel::index(int row, int column, const QModelIndex& parent /*= QModelIndex()*/) const
{
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }

    if (!parent.isValid())
    {
        return createIndex(row, column, row);
    }

    QModelIndex sourceParent;
    if (IsProxyRoot(parent))
    {
        int modelIdx = parent.row();
        sourceParent = GetRoot(modelIdx);
    }
    else
    {
        sourceParent = MapToSource(parent);
    }

    QModelIndex sourceIndex = sourceParent.model()->index(row, column, sourceParent);
    QModelIndex index = createIndex(row, column, sourceIndex.internalPointer());
    mappingToSource[index] = sourceIndex;
    //mappingFromSource[sourceIndex] = index;
    return index;
}

QModelIndex MultipleFileSystemModel::parent(const QModelIndex& child) const
{
    if (!child.isValid())
    {
        DVASSERT(false && "invalid child passed to parent function");
        return QModelIndex();
    }

    if (IsProxyRoot(child))
    {
        return QModelIndex();
    }

    auto sourceChild = MapToSource(child);
    auto sourceParent = sourceChild.model()->parent(sourceChild);

    if (sourceParent == GetRoot(sourceChild))
    {
        int row = GetModelIndex(sourceChild);
        return createIndex(row, 0, row);
    }

    return MapFromSource(sourceParent);
}

QModelIndex MultipleFileSystemModel::sibling(int row, int column, const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return QModelIndex();
    }

    if (IsProxyRoot(index))
    {
        return QModelIndex();
    }

    auto sourceIndex = MapToSource(index);
    return sourceIndex.model()->sibling(row, column, index);
}

int MultipleFileSystemModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    if (!parent.isValid())
    {
        return fileSystemModels.size();
    }

    if (IsProxyRoot(parent))
    {
        int modelIdx = parent.row();
        return GetModel(modelIdx)->rowCount(GetRoot(modelIdx));
    }

    auto sourceParent = MapToSource(parent);
    return sourceParent.model()->rowCount(sourceParent);
}

int MultipleFileSystemModel::columnCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    return 1;
}

QVariant MultipleFileSystemModel::data(const QModelIndex& index, int role /*= Qt::DisplayRole*/) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (IsProxyRoot(index))
    {
        int modelIdx = index.internalId();
        switch (role)
        {
        case Qt::DisplayRole:
            return GetAlias(modelIdx);
        case Qt::ToolTipRole:
            return GetModel(modelIdx)->rootPath();
        }

        return QVariant();
    }

    auto sourceIndex = MapToSource(index);
    return sourceIndex.model()->data(sourceIndex, role);
}

bool MultipleFileSystemModel::setData(const QModelIndex& index, const QVariant& value, int role /*= Qt::EditRole*/)
{
    if (!index.isValid())
    {
        return false;
    }

    if (IsProxyRoot(index))
    {
        return false;
    }

    auto sourceIndex = MapToSource(index);
    return GetModel(sourceIndex)->setData(sourceIndex, value, role);
}

QVariant MultipleFileSystemModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
    if (section == 0)
    {
        switch (role)
        {
        case Qt::DisplayRole:
            return QVariant("Name");
        default:
            return QVariant();
        }
    }
    return QVariant();
}

QMap<int, QVariant> MultipleFileSystemModel::itemData(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return QMap<int, QVariant>();
    }

    if (IsProxyRoot(index))
    {
        return QMap<int, QVariant>();
    }

    auto sourceIndex = MapToSource(index);
    return sourceIndex.model()->itemData(sourceIndex);
}

bool MultipleFileSystemModel::setItemData(const QModelIndex& index, const QMap<int, QVariant>& roles)
{
    if (!index.isValid())
    {
        return false;
    }

    if (IsProxyRoot(index))
    {
        return false;
    }

    auto sourceIndex = MapToSource(index);
    return GetModel(sourceIndex)->setItemData(sourceIndex, roles);
}

QStringList MultipleFileSystemModel::mimeTypes() const
{
    QStringList resultList;
    for (auto& info : fileSystemModels)
    {
        resultList << info.model->mimeTypes();
    }
    resultList.erase(std::unique(resultList.begin(), resultList.end()), resultList.end());
    return resultList;
}

QMimeData* MultipleFileSystemModel::mimeData(const QModelIndexList& indexes) const
{
    return nullptr;
}

bool MultipleFileSystemModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const
{
    if (!parent.isValid())
    {
        return false;
    }
    if (IsProxyRoot(parent))
    {
        return false;
    }

    auto sourceParent = MapToSource(parent);
    return sourceParent.model()->canDropMimeData(data, action, row, column, sourceParent);
}

bool MultipleFileSystemModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    if (!parent.isValid())
    {
        return false;
    }
    if (IsProxyRoot(parent))
    {
        return false;
    }

    auto sourceParent = MapToSource(parent);
    return GetModel(sourceParent)->dropMimeData(data, action, row, column, sourceParent);
}

Qt::DropActions MultipleFileSystemModel::supportedDropActions() const
{
    Qt::DropActions actions;
    for (auto& info : fileSystemModels)
    {
        Qt::DropActions action = info.model->supportedDropActions();
        actions = actions | action;
    }
    return actions;
}

bool MultipleFileSystemModel::hasChildren(const QModelIndex& parent /*= QModelIndex()*/) const
{
    if (!parent.isValid())
    {
        return true;
    }
    if (IsProxyRoot(parent))
    {
        return true;
    }

    auto sourceParent = MapToSource(parent);
    return sourceParent.model()->hasChildren(sourceParent);
}

void MultipleFileSystemModel::fetchMore(const QModelIndex& parent)
{
    if (!parent.isValid())
    {
        return;
    }
    if (IsProxyRoot(parent))
    {
        int modelIdx = parent.row();
        GetModel(modelIdx)->fetchMore(GetRoot(modelIdx));
        return;
    }

    auto sourceParent = MapToSource(parent);
    GetModel(sourceParent)->fetchMore(sourceParent);
}

bool MultipleFileSystemModel::canFetchMore(const QModelIndex& parent) const
{
    if (!parent.isValid())
    {
        return false;
    }
    if (IsProxyRoot(parent))
    {
        int modelIdx = parent.row();
        return GetModel(modelIdx)->canFetchMore(GetRoot(modelIdx));
    }

    auto sourceParent = MapToSource(parent);
    return sourceParent.model()->canFetchMore(sourceParent);
}

Qt::ItemFlags MultipleFileSystemModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return false;
    }
    if (IsProxyRoot(index))
    {
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }

    auto sourceIndex = MapToSource(index);
    return sourceIndex.model()->flags(sourceIndex);
}

void MultipleFileSystemModel::sort(int column, Qt::SortOrder order /*= Qt::AscendingOrder*/)
{
    for (auto& info : fileSystemModels)
    {
        info.model->sort(column, order);
    }
}

void MultipleFileSystemModel::source_dataChanged(const QModelIndex& sourceTopLeft, const QModelIndex& sourceBottomRight, const QVector<int>& roles)
{
    const QModelIndex topLeft = MapFromSource(sourceTopLeft);
    const QModelIndex bottomRight = MapFromSource(sourceBottomRight);
    emit dataChanged(topLeft, bottomRight, roles);
}

void MultipleFileSystemModel::source_rowsAboutToBeInserted(QModelIndex p, int from, int to)
{
    //QModelIndex p_p = mapFromSource(p);
    //    emit rowsAboutToBeInserted(p_p, from, to);
}

void MultipleFileSystemModel::source_rowsInserted(QModelIndex p, int from, int to)
{
    //QModelIndex p_p = mapFromSource(p);
    //    emit rowsAboutToBeInserted(p_p, from, to);
}

void MultipleFileSystemModel::source_rowsAboutToBeRemoved(QModelIndex, int, int)
{
}

void MultipleFileSystemModel::source_rowsRemoved(QModelIndex, int, int)
{
}

void MultipleFileSystemModel::ConnectToModel(QFileSystemModel* model)
{
    connect(model, &QFileSystemModel::rootPathChanged, this, &MultipleFileSystemModel::rootPathChanged);
    connect(model, &QFileSystemModel::fileRenamed, this, &MultipleFileSystemModel::fileRenamed);
    connect(model, &QFileSystemModel::directoryLoaded, this, &MultipleFileSystemModel::directoryLoaded);

    connect(model, &QFileSystemModel::layoutChanged, this, &MultipleFileSystemModel::layoutChanged);
    connect(model, &QFileSystemModel::layoutAboutToBeChanged, this, &MultipleFileSystemModel::layoutAboutToBeChanged);
    connect(model, &QFileSystemModel::modelReset, this, &MultipleFileSystemModel::modelReset);

    connect(model, &QFileSystemModel::dataChanged, this, &MultipleFileSystemModel::source_dataChanged);
    connect(model, &QFileSystemModel::rowsAboutToBeInserted, this, &MultipleFileSystemModel::source_rowsAboutToBeInserted);
    connect(model, &QFileSystemModel::rowsInserted, this, &MultipleFileSystemModel::source_rowsInserted);
    connect(model, &QFileSystemModel::rowsAboutToBeRemoved, this, &MultipleFileSystemModel::source_rowsAboutToBeRemoved);
    connect(model, &QFileSystemModel::rowsRemoved, this, &MultipleFileSystemModel::source_rowsRemoved);
}

void MultipleFileSystemModel::DisconnectFromModel(QFileSystemModel* model)
{
    disconnect(model, &QFileSystemModel::rootPathChanged, this, &MultipleFileSystemModel::rootPathChanged);
    disconnect(model, &QFileSystemModel::fileRenamed, this, &MultipleFileSystemModel::fileRenamed);
    disconnect(model, &QFileSystemModel::directoryLoaded, this, &MultipleFileSystemModel::directoryLoaded);

    disconnect(model, &QFileSystemModel::layoutChanged, this, &MultipleFileSystemModel::layoutChanged);
    disconnect(model, &QFileSystemModel::layoutAboutToBeChanged, this, &MultipleFileSystemModel::layoutAboutToBeChanged);
    disconnect(model, &QFileSystemModel::modelReset, this, &MultipleFileSystemModel::modelReset);

    disconnect(model, &QFileSystemModel::dataChanged, this, &MultipleFileSystemModel::source_dataChanged);
    disconnect(model, &QFileSystemModel::rowsAboutToBeInserted, this, &MultipleFileSystemModel::source_rowsAboutToBeInserted);
    disconnect(model, &QFileSystemModel::rowsInserted, this, &MultipleFileSystemModel::source_rowsInserted);
    disconnect(model, &QFileSystemModel::rowsAboutToBeRemoved, this, &MultipleFileSystemModel::source_rowsAboutToBeRemoved);
    disconnect(model, &QFileSystemModel::rowsRemoved, this, &MultipleFileSystemModel::source_rowsRemoved);
}

bool MultipleFileSystemModel::IsProxyRoot(const QModelIndex& parent) const
{
    return parent.internalId() < fileSystemModels.size();
}

const QModelIndex& MultipleFileSystemModel::MapToSource(const QModelIndex& proxyIndex) const
{
    static const QModelIndex dummyIndex;
    if (!proxyIndex.isValid())
    {
        DVASSERT(false);
        return dummyIndex;
    }

    if (IsProxyRoot(proxyIndex))
    {
        int modelIdx = proxyIndex.internalId();
        return GetRoot(modelIdx);
    }

    auto it = mappingToSource.find(proxyIndex);
    if (it != mappingToSource.end())
    {
        return it.value();
    }

    DVASSERT(false);
    return dummyIndex;
}

const QModelIndex& MultipleFileSystemModel::MapFromSource(const QModelIndex& sourceIndex) const
{
    DVASSERT(sourceIndex.isValid());

    return createIndex(sourceIndex.row(), sourceIndex.column(), sourceIndex.internalPointer());
    //     auto it = mappingFromSource.find(sourceIndex);
    //     if (it != mappingFromSource.end())
    //     {
    //         return it.value();
    //     }
    //     static const QModelIndex dummyIndex;
    //     DVASSERT(false);
    //     return dummyIndex;
}

QFileSystemModel* MultipleFileSystemModel::GetModel(const QModelIndex& sourceIndex) const
{
    return GetInfo(sourceIndex).model;
}

const QModelIndex& MultipleFileSystemModel::GetRoot(const QModelIndex& sourceIndex) const
{
    return GetInfo(sourceIndex).root;
}

int MultipleFileSystemModel::GetModelIndex(const QModelIndex& index) const
{
    int modelIdx = 0;
    for (auto& info : fileSystemModels)
    {
        if (info.model == index.model())
        {
            return modelIdx;
        }
        ++modelIdx;
    }

    return -1;
}

const MultipleFileSystemModel::FileSystemInfo& MultipleFileSystemModel::GetInfo(const QModelIndex& sourceIndex) const
{
    for (auto& info : fileSystemModels)
    {
        if (info.model == sourceIndex.model())
        {
            return info;
        }
    }

    DVASSERT(false);
    static const FileSystemInfo dummy;
    return dummy;
}
