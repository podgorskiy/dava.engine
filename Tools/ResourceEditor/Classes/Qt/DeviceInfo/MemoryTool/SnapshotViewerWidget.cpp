#include "Debug/DVAssert.h"
#include "Platform/SystemTimer.h"

#include "Qt/DeviceInfo/MemoryTool/SnapshotViewerWidget.h"

#include "Qt/DeviceInfo/MemoryTool/Models/SymbolsListModel.h"
#include "Qt/DeviceInfo/MemoryTool/Models/BranchTreeModel.h"
#include "Qt/DeviceInfo/MemoryTool/Models/BlockListModel.h"

#include "Qt/DeviceInfo/MemoryTool/Branch.h"
#include "Qt/DeviceInfo/MemoryTool/ProfilingSession.h"
#include "Qt/DeviceInfo/MemoryTool/MemorySnapshot.h"
#include "Qt/DeviceInfo/MemoryTool/SymbolsWidget.h"
#include "Qt/DeviceInfo/MemoryTool/MemoryBlocksWidget.h"

#include <QDebug>
#include <QFileDialog>
#include <QTabWidget>
#include <QTreeView>
#include <QVBoxLayout>
#include <QAction>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QLineEdit>
#include <QListView>
#include <QComboBox>
#include <QStandardItemModel>
#include <QStandardItem>

using namespace DAVA;

SnapshotViewerWidget::SnapshotViewerWidget(const ProfilingSession* session_, size_t snapshotIndex, QWidget* parent)
    : QWidget(parent, Qt::Window)
    , session(session_)
{
    DVASSERT(session != nullptr);

    snapshot = &session->Snapshot(snapshotIndex);

    allBlocksLinked = BlockLink::CreateBlockLink(snapshot);

    Init();
}

SnapshotViewerWidget::~SnapshotViewerWidget()
{
    delete branchTreeModel;
    delete blockListModel;
}

void SnapshotViewerWidget::Init()
{
    tab = new QTabWidget;

    InitSymbolsView();
    InitBranchView();
    InitMemoryBlocksView();

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tab);
    setLayout(mainLayout);
}

void SnapshotViewerWidget::InitSymbolsView()
{
    symbolWidget = new SymbolsWidget(*snapshot->SymbolTable());
    QPushButton* buildTree = new QPushButton("Build tree");
    connect(buildTree, &QPushButton::clicked, this, &SnapshotViewerWidget::SymbolView_OnBuldTree);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(buildTree);
    layout->addWidget(symbolWidget);

    QFrame* frame = new QFrame;
    frame->setLayout(layout);
    tab->addTab(frame, "Symbols");
}

void SnapshotViewerWidget::InitBranchView()
{
    branchTreeModel = new BranchTreeModel(snapshot);
    branchFilterModel = new BranchFilterModel;
    branchFilterModel->setSourceModel(branchTreeModel);

    blockListModel = new BlockListModel;

    branchTree = new QTreeView;
    branchTree->setFont(QFont("Consolas", 10, 500));
    //branchTree->setModel(branchTreeModel);
    branchTree->setModel(branchFilterModel);

    blockList = new QListView;
    blockList->setFont(QFont("Consolas", 10, 500));
    blockList->setModel(blockListModel);

    QItemSelectionModel* selModel = branchTree->selectionModel();
    connect(selModel, &QItemSelectionModel::currentChanged, this, &SnapshotViewerWidget::BranchView_SelectionChanged);
    connect(blockList, &QTreeView::doubleClicked, this, &SnapshotViewerWidget::BranchBlockView_DoubleClicked);

    QPushButton* apply = new QPushButton("Apply");
    connect(apply, &QPushButton::clicked, this, &SnapshotViewerWidget::ApplyClicked);

    QHBoxLayout* hl = new QHBoxLayout;
    hl->addWidget(InitAllocPoolsCombo());
    hl->addWidget(InitTagsCombo());
    hl->addWidget(apply);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addLayout(hl);
    layout->addWidget(branchTree);
    layout->addWidget(blockList);

    QFrame* frame = new QFrame;
    frame->setLayout(layout);

    tab->addTab(frame, "Branches");
}

void SnapshotViewerWidget::InitMemoryBlocksView()
{
    memoryBlocksWidget = new MemoryBlocksWidget(session, &allBlocksLinked);

    tab->addTab(memoryBlocksWidget, "Memory blocks");
}

QComboBox* SnapshotViewerWidget::InitAllocPoolsCombo()
{
    const ProfilingSession* session = snapshot->Session();

    int nrows = static_cast<int>(session->AllocPoolCount());
    QStandardItemModel* model = new QStandardItemModel(nrows, 1);
    for (int i = 0;i < nrows;++i)
    {
        const String& name = session->AllocPoolName(i);
        QStandardItem* item = new QStandardItem(QString(name.c_str()));
        item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        item->setData(Qt::Unchecked, Qt::CheckStateRole);
        item->setData(1 << i, Qt::UserRole + 1);

        model->setItem(i, 0, item);
    }
    connect(model, &QStandardItemModel::itemChanged, this, &SnapshotViewerWidget::AllocPoolComboItemChanged);

    QComboBox* widget = new QComboBox;
    widget->setModel(model);
    return widget;
}

QComboBox* SnapshotViewerWidget::InitTagsCombo()
{
    const ProfilingSession* session = snapshot->Session();

    int nrows = static_cast<int>(session->TagCount());
    QStandardItemModel* model = new QStandardItemModel(nrows, 1);
    for (int i = 0;i < nrows;++i)
    {
        const String& name = session->TagName(i);
        QStandardItem* item = new QStandardItem(QString(name.c_str()));
        item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        item->setData(Qt::Unchecked, Qt::CheckStateRole);
        item->setData(1 << i, Qt::UserRole + 1);

        model->setItem(i, 0, item);
    }
    connect(model, &QStandardItemModel::itemChanged, this, &SnapshotViewerWidget::TagComboItemChanged);

    QComboBox* widget = new QComboBox;
    widget->setModel(model);
    return widget;
}

void SnapshotViewerWidget::SymbolView_OnBuldTree()
{
    Vector<const String*> selection = symbolWidget->GetSelectedSymbols();
    if (!selection.empty())
    {
        branchTreeModel->PrepareModel(selection);
    }
}

void SnapshotViewerWidget::BranchView_SelectionChanged(const QModelIndex& current, const QModelIndex& previous)
{
    QModelIndex index = branchFilterModel->mapToSource(current);

    Branch* branch = static_cast<Branch*>(index.internalPointer());
    if (branch != nullptr)
    {
        Vector<MMBlock*> blocks = branch->GetMemoryBlocks();
        blockListModel->PrepareModel(std::forward<Vector<MMBlock*>>(blocks));
    }
}

void SnapshotViewerWidget::BranchBlockView_DoubleClicked(const QModelIndex& current)
{
    const MMBlock* block = blockListModel->GetBlock(current);
    if (block != nullptr)
    {
        // TODO: expand callstack tree to view block allocation site
    }
}

void SnapshotViewerWidget::AllocPoolComboItemChanged(QStandardItem* item)
{
    int chk = item->data(Qt::CheckStateRole).toInt();
    int v = item->data(Qt::UserRole + 1).toInt();
    if (chk == Qt::Checked)
    {
        poolFilter |= v;
    }
    else if (chk == Qt::Unchecked)
    {
        poolFilter &= ~v;
    }
}

void SnapshotViewerWidget::TagComboItemChanged(QStandardItem* item)
{
    int chk = item->data(Qt::CheckStateRole).toInt();
    int v = item->data(Qt::UserRole + 1).toInt();
    if (chk == Qt::Checked)
    {
        tagFilter |= v;
    }
    else if (chk == Qt::Unchecked)
    {
        tagFilter &= ~v;
    }
}

void SnapshotViewerWidget::ApplyClicked()
{
    branchFilterModel->SetFilter(poolFilter, tagFilter);
}
