#include "LogWidget.h"
#include <QDebug>
#include <QClipboard>
#include <QKeyEvent>
#include <QScrollBar>
#include <QBuffer>

#include "LogModel.h"
#include "LogFilterModel.h"
#include "LogDelegate.h"

namespace
{
    const int scrollDelay = 50;
}


LogWidget::LogWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUi(this);
    time.start();

    LogDelegate* delegate = new LogDelegate(log, this);
    logModel = new LogModel(this);
    logFilterModel = new LogFilterModel(this);

    logFilterModel->setSourceModel(logModel);
    log->setModel(logFilterModel);
    connect(delegate, &LogDelegate::copyRequest, this, &LogWidget::OnCopy);
    connect(delegate, &LogDelegate::clearRequest, this, &LogWidget::OnClear);
    log->installEventFilter(this);

    FillFiltersCombo();

    connect(filter, &CheckableComboBox::selectedUserDataChanged, logFilterModel, &LogFilterModel::SetFilters);
    connect(search, &LineEditEx::textUpdated, this, &LogWidget::OnTextFilterChanged);
    filter->selectUserData(logFilterModel->GetFilters());
}

LogModel* LogWidget::Model()
{
    return logModel;
}

QByteArray LogWidget::Serialize() const
{
    QByteArray retData;
    QDataStream stream(&retData, QIODevice::WriteOnly);
    stream << logFilterModel->GetFilterString();
    stream << logFilterModel->GetFilters();
    return retData;
}

void LogWidget::Deserialize(const QByteArray& data)
{
    QDataStream stream(data);
    QString filterString;
    stream >> filterString;
    logFilterModel->SetFilterString(filterString);
    QVariantList logLevels;
    stream >> logLevels;
    logFilterModel->SetFilters(logLevels);
    filter->selectUserData(logLevels);
}

void LogWidget::OnTextFilterChanged(const QString& text)
{
    logFilterModel->SetFilterString(text);
}

void LogWidget::FillFiltersCombo()
{
    filter->addItem("LEVEL_FRAMEWORK", DAVA::Logger::LEVEL_FRAMEWORK);
    filter->addItem("LEVEL_DEBUG", DAVA::Logger::LEVEL_DEBUG);
    filter->addItem("LEVEL_INFO", DAVA::Logger::LEVEL_INFO);
    filter->addItem("LEVEL_WARNING", DAVA::Logger::LEVEL_WARNING);
    filter->addItem("LEVEL_ERROR", DAVA::Logger::LEVEL_ERROR);

    QAbstractItemModel* m = filter->model();
    const int n = m->rowCount();
    for (int i = 0; i < n; i++)
    {
        QModelIndex index = m->index(i, 0, QModelIndex());
        const int ll = index.data(LogModel::LEVEL_ROLE).toInt();
        const QPixmap& pix = logModel->GetIcon(ll);
        m->setData(index, pix, Qt::DecorationRole);
        m->setData(index, Qt::Checked, Qt::CheckStateRole);
    }
}


bool LogWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == log)
    {
        switch (event->type())
        {
        case QEvent::KeyPress:
            {
                QKeyEvent* ke = static_cast<QKeyEvent *>(event);
                if (ke->matches(QKeySequence::Copy))
                {
                    OnCopy();
                    return true;
                }
            }
            break;

        default:
            break;
        }
    }

    return QWidget::eventFilter(watched, event);
}

void LogWidget::OnCopy()
{
    const QModelIndexList& selection = log->selectionModel()->selectedIndexes();
    const int n = selection.size();
    if (n == 0)
        return ;

    QMap< int, QModelIndex > sortedSelection;
    for (int i = 0; i < n; i++)
    {
        const QModelIndex& index = selection[i];
        const int realIdx = index.row();
        sortedSelection[realIdx] = index;
    }

    QString text;
    QTextStream ss(&text);
    for (auto it = sortedSelection.constBegin(); it != sortedSelection.constEnd(); ++it)
    {
        ss << it.value().data(Qt::DisplayRole).toString() << "\n";
    }
    ss.flush();

    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(text);
}

void LogWidget::OnClear()
{
    logModel->clear();
}
