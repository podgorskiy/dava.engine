#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "RemoteAssetCacheServer.h"
#include <FileSystem/KeyedArchive.h>

#include <QFileDialog>
#include <QMenu>
#include <QCloseEvent>
#include <QSettings>
#include <QTimer>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , trayIcon(nullptr)
{
    ui->setupUi(this);

    connect(ui->addNewServerButton, &QPushButton::clicked, this, &MainWindow::OnAddNewServerWidget);
    connect(ui->selectFolderButton, &QPushButton::clicked, this, &MainWindow::OnSelectFolder);
    connect(ui->clearDirectoryButton, &QPushButton::clicked, ui->cachFolderLineEdit, &QLineEdit::clear);

    connect(ui->cachFolderLineEdit, &QLineEdit::textChanged, this, &MainWindow::CheckEnableClearButton);

    connect(ui->saveButton, &QPushButton::clicked, this, &MainWindow::OnSaveButtonClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &MainWindow::OnCancelButtonClicked);

    ReadSettings();

    ShowTrayIcon();
}

MainWindow::~MainWindow()
{
    WriteSettings();
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    hide();
    e->ignore();
    openAction->setEnabled(true);
}

void MainWindow::OnAddNewServerWidget()
{
    RemoteAssetCacheServer *server = new RemoteAssetCacheServer(this);
    servers << server;

    connect(server, &RemoteAssetCacheServer::RemoveLater, this, &MainWindow::OnRemoveServerWidget);

    ui->serversBox->layout()->addWidget(server);
    emit NewServerAdded(server->GetServerData());
}

void MainWindow::OnRemoveServerWidget()
{
    RemoteAssetCacheServer *w = qobject_cast<RemoteAssetCacheServer *>(sender());
    servers.removeOne(w);
    emit ServerRemoved(w->GetServerData());
    w->deleteLater();
    QTimer::singleShot(10, [this]{adjustSize();});
    adjustSize();
}

void MainWindow::OnSelectFolder()
{
    QString directory = QFileDialog::getExistingDirectory(this, "Choose directory", QDir::currentPath(),
                                                          QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui->cachFolderLineEdit->setText(directory);
    emit FolderChanged(directory);
}

void MainWindow::CheckEnableClearButton()
{
    ui->clearDirectoryButton->setEnabled(!ui->cachFolderLineEdit->text().isEmpty());
}

void MainWindow::OnTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
        case QSystemTrayIcon::DoubleClick:
        {
            if (!this->isVisible())
            {
                this->show();
            }
            break;
        }
        default:
            break;
    }
}

void MainWindow::OnServerParametersChanged()
{
    QVector<ServerData> serversData;
    for (auto &server : servers)
    {
        serversData << server->GetServerData();
    }
    emit ServersChanged(serversData);
}

void MainWindow::OnOpenAction()
{
    if (!this->isVisible())
    {
        this->show();
        openAction->setEnabled(false);
    }
}

void MainWindow::OnSaveButtonClicked()
{
    WriteSettings();
    this->hide();
}

void MainWindow::OnCancelButtonClicked()
{
    this->hide();
}

void MainWindow::SetFolder(QString &folderPath)
{
    if (QString::compare(folderPath, ui->cachFolderLineEdit->text(), Qt::CaseInsensitive) != 0)
    {
        ui->cachFolderLineEdit->setText(folderPath);
    }
}

void MainWindow::SetFolderSize(qreal folderSize)
{
    ui->cachSizeSpinBox->setValue(folderSize);
}

void MainWindow::SetFilesCount(quint32 filesCounts)
{
    if (ui->numberOfFilesSpinBox->value() != filesCounts)
    {
        ui->numberOfFilesSpinBox->setValue(filesCounts);
    }
}

void MainWindow::AddServers(QVector<ServerData> &newServers)
{
    for (auto &newServer : newServers)
    {
        AddServer(newServer);
    }
}

void MainWindow::AddServer(ServerData newServer)
{
    RemoteAssetCacheServer *server = new RemoteAssetCacheServer(newServer, this);
    servers << server;

    connect(server, &RemoteAssetCacheServer::RemoveLater, this, &MainWindow::OnRemoveServerWidget);

    ui->serversBox->layout()->addWidget(server);

    emit NewServerAdded(server->GetServerData());
}

void MainWindow::CreateTrayIconActions()
{
    QAction *quitAction = new QAction("Quit", this);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    openAction = new QAction("Open server", this);
    openAction->setEnabled(false);
    connect(openAction, &QAction::triggered, this, &MainWindow::OnOpenAction);

    trayActionsMenu = new QMenu(this);
    trayActionsMenu->addAction(openAction);
    trayActionsMenu->addSeparator();
    trayActionsMenu->addAction(quitAction);
}

void MainWindow::ShowTrayIcon()
{
    trayIcon = new QSystemTrayIcon(this);
    QIcon trayImage(":/icon/TrayIcon.png");
    trayIcon->setIcon(trayImage);

    CreateTrayIconActions();
    trayIcon->setContextMenu(trayActionsMenu);

    trayIcon->setToolTip("Asset Cache Server");

    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::OnTrayIconActivated);

    trayIcon->show();
}

void MainWindow::ReadSettings()
{
    using namespace DAVA;

    FilePath path("~doc:/AssetServer/ACS_settings.dat");
    File *f = File::Create(path, File::OPEN | File::READ);
    if (f == nullptr)
    {
        return;
    }

    KeyedArchive *arch = new KeyedArchive();
    arch->Load(f);

    String folderPath = arch->GetString(String("FolderPath"));
    ui->cachFolderLineEdit->setText(folderPath.c_str());

    DAVA::float32 folderSize = arch->GetFloat(String("FolderSize"));
    ui->cachSizeSpinBox->setValue(folderSize);

    DAVA::uint32 numberOfFiles = arch->GetUInt32(String("NumberOfFiles"));
    ui->numberOfFilesSpinBox->setValue(numberOfFiles);

    bool startup = arch->GetBool(String("Startup"));
    ui->startupCheckBox->setChecked(startup);

    DAVA::uint32 port = arch->GetUInt32(String("Port"));
    ui->portSpinBox->setValue(port);

    auto size = arch->GetUInt32("ServersSize");
    for (int i = 0; i < size; ++i)
    {
        String ip = arch->GetString(Format("Server_%d_ip", i));
        DAVA::uint32 port = arch->GetUInt32(Format("Server_%d_port", i));
        ServerData sData(QString(ip.c_str()), static_cast<quint16>(port));
        AddServer(sData);
    }

    f->Release();
    arch->Release();
}

#include "FileSystem/FileSystem.h"
void MainWindow::WriteSettings()
{
    using namespace DAVA;

    FileSystem::Instance()->CreateDirectoryA("~doc:/AssetServer", true);
    FilePath path("~doc:/AssetServer/ACS_settings.dat");
    File *f = File::Create(path, File::CREATE | File::WRITE);
    if (f == nullptr)
    {
        return;
    }

    KeyedArchive *arch = new KeyedArchive();

    arch->SetString(String("FolderPath"), String(ui->cachFolderLineEdit->text().toStdString()));
    arch->SetFloat(String("FolderSize"), static_cast<DAVA::float32>(ui->cachSizeSpinBox->value()));
    arch->SetUInt32(String("NumberOfFiles"), static_cast<DAVA::uint32>(ui->numberOfFilesSpinBox->value()));
    arch->SetBool(String("Startup"), ui->startupCheckBox->isChecked());
    arch->SetUInt32(String("Port"), static_cast<DAVA::uint32>(ui->portSpinBox->value()));

    auto size = servers.size();
    arch->SetUInt32("ServersSize", size);

    for (int i = 0; i < size; ++i)
    {
        auto &sData = servers.at(i)->GetServerData();
        arch->SetString(Format("Server_%d_ip", i), String(sData.ip.toStdString()));
        arch->SetUInt32(Format("Server_%d_port", i), static_cast<DAVA::uint32>(sData.port));
    }

    arch->Save(f);
    f->Release();
    arch->Release();
}
