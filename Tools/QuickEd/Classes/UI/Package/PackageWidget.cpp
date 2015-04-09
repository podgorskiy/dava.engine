
#include <QClipboard>
#include "PackageWidget.h"
#include "PackageModel.h"

#include "UI/QtModelPackageCommandExecutor.h"

#include "UI/Package/FilteredPackageModel.h"
#include "Model/PackageHierarchy/PackageBaseNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ImportedPackagesNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/PackageRef.h"
#include "Model/YamlPackageSerializer.h"
#include "Model/EditorUIPackageBuilder.h"

#include "Project.h"
#include "Utils/QtDavaConvertion.h"

#include "SharedData.h"
#include "Document.h"

using namespace DAVA;

namespace
{
    struct PackageContext : public WidgetContext
    {
        PackageContext(Document *document)
        {
            DVASSERT(document);
            packageModel = new PackageModel(document->GetPackage(), document->GetCommandExecutor(), document);
            filteredPackageModel = new FilteredPackageModel(document);
            filteredPackageModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
            filteredPackageModel->setSourceModel(packageModel);
        }
        PackageModel *packageModel;
        FilteredPackageModel *filteredPackageModel;
        QList<QPersistentModelIndex> expandedIndexes;
        QItemSelection selection;
        QString filterString;
    };
}

PackageWidget::PackageWidget(QWidget *parent)
    : QDockWidget(parent)
    , sharedData(nullptr)
    , filteredPackageModel(nullptr)
    , packageModel(nullptr)
{
    setupUi(this);
    treeView->header()->setSectionResizeMode/*setResizeMode*/(QHeaderView::ResizeToContents);

    connect(filterLine, &QLineEdit::textChanged, this, &PackageWidget::filterTextChanged);

    importPackageAction = new QAction(tr("Import package"), this);

    cutAction = new QAction(tr("Cut"), this);
    cutAction->setShortcut(QKeySequence(QKeySequence::Cut));
    cutAction->setShortcutContext(Qt::WidgetShortcut);
    connect(cutAction, &QAction::triggered, this, &PackageWidget::OnCut);

    copyAction = new QAction(tr("Copy"), this);
    copyAction->setShortcut(QKeySequence(QKeySequence::Copy));
    copyAction->setShortcutContext(Qt::WidgetShortcut);
    connect(copyAction, &QAction::triggered, this, &PackageWidget::OnCopy);

    pasteAction = new QAction(tr("Paste"), this);
    pasteAction->setShortcut(QKeySequence(QKeySequence::Paste));
    pasteAction->setShortcutContext(Qt::WidgetShortcut);
    connect(pasteAction, &QAction::triggered, this, &PackageWidget::OnPaste);

    delAction = new QAction(tr("Delete"), this);
    delAction->setShortcut(QKeySequence(QKeySequence::Delete));
    delAction->setShortcutContext(Qt::WidgetShortcut);
    connect(delAction, &QAction::triggered, this, &PackageWidget::OnDelete);

    treeView->addAction(importPackageAction);
    treeView->addAction(copyAction);
    treeView->addAction(pasteAction);
    treeView->addAction(cutAction);
    treeView->addAction(delAction);
}

void PackageWidget::OnDocumentChanged(SharedData *context)
{
    treeView->setUpdatesEnabled(false);

    SaveContext();
    sharedData = context;

    LoadContext();

    treeView->setColumnWidth(0, treeView->size().width()); // TODO Check this
    treeView->setUpdatesEnabled(true);
}

void PackageWidget::OnDataChanged(const QByteArray &role)
{
    if (role == "selectedNodes")
    {
        OnControlSelectedInEditor(sharedData->GetData("selectedNodes").value<QList<ControlNode*> >());
    }
}

void PackageWidget::LoadContext()
{
    delete treeView->selectionModel();
    if (nullptr == sharedData)
    {
        treeView->setModel(nullptr);
        packageModel = nullptr;
        filteredPackageModel = nullptr;
    }
    else
    {
        //restore context
        PackageContext *context = reinterpret_cast<PackageContext*>(sharedData->GetContext(this));
        if (nullptr == context)
        {
            context = new PackageContext(qobject_cast<Document*>(sharedData->parent()));
            sharedData->SetContext(this, context);
        }
        //store model to work with indexes
        packageModel = context->packageModel;
        filteredPackageModel = context->filteredPackageModel;
        //remove it in future
        sharedData->SetData("packageModel", QVariant::fromValue<QAbstractItemModel*>(context->packageModel)); //TODO: bad architecture
        //restore model
        treeView->setModel(context->filteredPackageModel);
        treeView->expandToDepth(0);
        connect(treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PackageWidget::OnSelectionChanged);
        //restore expanded indexes
        for (const auto &index : context->expandedIndexes)
        {
            if (index.isValid())
            {
                treeView->setExpanded(index, true);
            }
        }
        //restore selection
        treeView->selectionModel()->select(context->selection, QItemSelectionModel::ClearAndSelect);
        //restore filter line
        filterLine->setText(context->filterString);
    }

}

void PackageWidget::SaveContext()
{
    if (nullptr == sharedData)
    {
        return;
    }
    PackageContext *context = reinterpret_cast<PackageContext*>(sharedData->GetContext(this));
    context->expandedIndexes = GetExpandedIndexes();
    context->selection = treeView->selectionModel()->selection();
    context->filterString = filterLine->text();
}

void PackageWidget::RefreshActions(const QModelIndexList &indexList)
{
    bool canInsert = !indexList.empty();
    bool canRemove = !indexList.empty();
    bool canCopy = !indexList.empty();
    
    for(const auto &index : indexList)
    {
        PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
        if (!node->CanCopy())
            canCopy = false;

        if (!node->IsInsertingSupported())
            canInsert = false;

        if (!node->CanRemove())
            canRemove = false;
    }
    
    RefreshAction(copyAction, canCopy, true);
    RefreshAction(pasteAction, canInsert, true);
    RefreshAction(cutAction, canCopy && canRemove, true);
    RefreshAction(delAction, canRemove, true);

    RefreshAction(importPackageAction, false, false);

}

void PackageWidget::RefreshAction( QAction *action, bool enabled, bool visible )
{
    action->setDisabled(!enabled);
    action->setVisible(visible);
}

void PackageWidget::CollectSelectedNodes(Vector<ControlNode*> &nodes)
{
    QItemSelection selected = filteredPackageModel->mapSelectionToSource(treeView->selectionModel()->selection());
    QModelIndexList selectedIndexList = selected.indexes();
    
    if (!selectedIndexList.empty())
    {
        for (QModelIndex &index : selectedIndexList)
        {
            PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
            ControlNode *controlNode = dynamic_cast<ControlNode*>(node);
            
            if (controlNode && controlNode->CanCopy())
                nodes.push_back(controlNode);
        }
    }
}

void PackageWidget::CopyNodesToClipboard(const DAVA::Vector<ControlNode*> &nodes)
{
    QClipboard *clipboard = QApplication::clipboard();
    if (!nodes.empty())
    {
        YamlPackageSerializer serializer;
        Document* doc = sharedData->GetDocument();
        PackageNode *pac = doc->GetPackage();
        pac->Serialize(&serializer, nodes);//TODO - this is deprecated
        String str = serializer.WriteToString();
        QMimeData data;
        data.setText(QString(str.c_str()));
        clipboard->setMimeData(&data);
    }
}

void PackageWidget::RemoveNodes(const DAVA::Vector<ControlNode*> &nodes)
{
    sharedData->GetDocument()->GetCommandExecutor()->RemoveControls(nodes);
}

void PackageWidget::OnSelectionChanged(const QItemSelection &proxySelected, const QItemSelection &proxyDeselected)
{
    if (filteredPackageModel.isNull())
    {
        return;
    }
    QList<ControlNode*> selectedRootControl;
    QList<ControlNode*> deselectedRootControl;
    
    QList<ControlNode*> selectedControl;
    QList<ControlNode*> deselectedControl;

    QItemSelection selected = filteredPackageModel->mapSelectionToSource(proxySelected);
    QItemSelection deselected = filteredPackageModel->mapSelectionToSource(proxyDeselected);

    QModelIndexList selectedIndexList = selected.indexes();

    if (!selectedIndexList.empty())
    {
        for(QModelIndex &index : selectedIndexList)
        {
            PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
            if (node->GetControl())
            {
                selectedControl.push_back(static_cast<ControlNode*>(node));
                
                while (node->GetParent() && node->GetParent()->GetControl())
                    node = node->GetParent();
                
                if (selectedRootControl.indexOf(static_cast<ControlNode*>(node)) < 0)
                    selectedRootControl.push_back(static_cast<ControlNode*>(node));
            }
        }
    }

    QModelIndexList deselectedIndexList = deselected.indexes();
    if (!deselectedIndexList.empty())
    {
        for (QModelIndex &index : deselectedIndexList)
        {
            PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
            if (node->GetControl())
            {
                deselectedControl.push_back(static_cast<ControlNode*>(node));

                while (node->GetParent() && node->GetParent()->GetControl())
                    node = node->GetParent();

                if (deselectedRootControl.indexOf(static_cast<ControlNode*>(node)) < 0)
                    deselectedRootControl.push_back(static_cast<ControlNode*>(node));
            }
        }
    }

    RefreshActions(selectedIndexList);
    sharedData->SetData("activatedControls", QVariant::fromValue(selectedControl));
    sharedData->SetData("deactivatedControls", QVariant::fromValue(deselectedControl));
}

void PackageWidget::OnCopy()
{
    Vector<ControlNode*> nodes;
    CollectSelectedNodes(nodes);
    CopyNodesToClipboard(nodes);
}

void PackageWidget::OnPaste()
{
    QItemSelection selected = filteredPackageModel->mapSelectionToSource(treeView->selectionModel()->selection());
    QModelIndexList selectedIndexList = selected.indexes();
    QClipboard *clipboard = QApplication::clipboard();
    
    if (!selectedIndexList.empty() && clipboard && clipboard->mimeData())
    {
        QModelIndex &index = selectedIndexList.first();
        
        ControlsContainerNode *node = dynamic_cast<ControlsContainerNode*>(static_cast<PackageBaseNode*>(index.internalPointer()));
        
        if (nullptr != node && (node->GetFlags() & PackageBaseNode::FLAG_READ_ONLY) == 0)
        {
            String string = clipboard->mimeData()->text().toStdString();
            Document *doc = sharedData->GetDocument();
            doc->GetCommandExecutor()->Paste(doc->GetPackage(), node, -1, string);
        }
    }
}

void PackageWidget::OnCut()
{
    Vector<ControlNode*> nodes;
    CollectSelectedNodes(nodes);
    CopyNodesToClipboard(nodes);
    RemoveNodes(nodes);
}

void PackageWidget::OnDelete()
{
    Vector<ControlNode*> nodes;
    CollectSelectedNodes(nodes);
    RemoveNodes(nodes);
}

void PackageWidget::filterTextChanged(const QString &filterText)
{
    if (nullptr != sharedData)
    {
        static_cast<QSortFilterProxyModel*>(treeView->model())->setFilterFixedString(filterText);
        treeView->expandAll();
    }
}

void PackageWidget::OnControlSelectedInEditor(const QList<ControlNode *> &selectedNodes)
{
    if (selectedNodes.isEmpty())
    {
        treeView->selectionModel()->clear();
    }
    else
    {
        for (auto &node : selectedNodes)
        {
            QModelIndex srcIndex = packageModel->indexByNode(node);
            QModelIndex dstIndex = filteredPackageModel->mapFromSource(srcIndex);
            treeView->selectionModel()->select(dstIndex, QItemSelectionModel::ClearAndSelect);
            treeView->expand(dstIndex);
            treeView->scrollTo(dstIndex);
        }
    }
}

QList<QPersistentModelIndex> PackageWidget::GetExpandedIndexes() const
{
    QList<QPersistentModelIndex> retval;
    QModelIndex index = treeView->model()->index(0, 0);
    while (index.isValid())
    {
        if (treeView->isExpanded(index))
        {
            retval << QPersistentModelIndex(index);
        }
        index = treeView->indexBelow(index);
    }

    return retval;
}
