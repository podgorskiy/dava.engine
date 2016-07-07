#include "FileSystem/FileSystem.h"

#include "QEApplication.h"
#include "EditorCore.h"

#include "QtTools/Utils/Themes/Themes.h"

#include "NgtTools/Commands/WGTCommand.h"
#include "NgtTools/Common/GlobalContext.h"

#include <core_command_system/i_command_manager.hpp>
#include <core_command_system/i_history_panel.h>
#include <QtTools/DavaGLWidget/davaglwidget.h>

QEApplication::QEApplication(int argc, char** argv)
    : BaseApplication(argc, argv)
    , ngtCommand(new WGTCommand())
{
}

QEApplication::~QEApplication() = default;

int QEApplication::Run()
{
    wgt::IHistoryPanel* historyPanel = NGTLayer::queryInterface<wgt::IHistoryPanel>();
    if (historyPanel != nullptr)
    {
        historyPanel->setClearButtonVisible(false);
        historyPanel->setMakeMacroButtonVisible(false);
    }
    editorCore = new EditorCore();

    editorCore->Start();
    int exitCode = StartApplication(editorCore->GetMainWindow());
    delete editorCore;
    editorCore = nullptr;
    return exitCode;
}

void QEApplication::GetPluginsForLoad(DAVA::Vector<DAVA::WideString>& names) const
{
    names.push_back(L"plg_variant");
    names.push_back(L"plg_reflection");
    names.push_back(L"plg_command_system");
    names.push_back(L"plg_serialization");
    names.push_back(L"plg_file_system");
    names.push_back(L"plg_editor_interaction");
    names.push_back(L"plg_qt_app");
    names.push_back(L"plg_qt_common");
    //uncomment it when DavaGLWidget will be rewrited to the QQuickWidget
    //names.push_back(L"plg_history_ui");
}

void QEApplication::OnPostLoadPlugins()
{
    qApp->setOrganizationName("DAVA");
    qApp->setApplicationName("QuickEd");

    commandManager = NGTLayer::queryInterface<wgt::ICommandManager>();
    commandManager->SetHistorySerializationEnabled(false);
    commandManager->registerCommand(ngtCommand.get());

    const char* settingsPath = "QuickEdSettings.archive";
    DAVA::FilePath localPrefrencesPath(DAVA::FileSystem::Instance()->GetCurrentDocumentsDirectory() + settingsPath);
    PreferencesStorage::Instance()->SetupStoragePath(localPrefrencesPath);

    Themes::InitFromQApplication();

    BaseApplication::OnPostLoadPlugins();
}

void QEApplication::OnPreUnloadPlugins()
{
    commandManager->deregisterCommand(ngtCommand->getId());
    BaseApplication::OnPreUnloadPlugins();
}

void QEApplication::ConfigureLineCommand(NGTLayer::NGTCmdLineParser& lineParser)
{
    lineParser.addParam("preferenceFolder", DAVA::FileSystem::Instance()->GetCurrentDocumentsDirectory().GetAbsolutePathname() + "QuickEd/");
}

bool QEApplication::OnRequestCloseApp()
{
    DVASSERT(editorCore != nullptr);
    return editorCore->CloseProject();
}
