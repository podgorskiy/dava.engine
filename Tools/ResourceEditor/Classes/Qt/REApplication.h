#pragma once

#include "CommandLine/CommandLineManager.h"
#include <QApplication>

class WGTCommand;

namespace NGTLayer
{
class NGTCmdLineParser;
}

namespace wgt
{
class ICommandManager;
}

class REApplication : public QApplication
{
public:
    REApplication(int argc, char** argv);
    ~REApplication();

    int Run();

protected:
    void GetPluginsForLoad(DAVA::Vector<DAVA::WideString>& names) const;
    void OnPostLoadPlugins();
    bool OnRequestCloseApp();

private:
    void RunWindow();
    void RunConsole();

private:
    QtMainWindow* mainWindow = nullptr;
    std::unique_ptr<CommandLineManager> cmdLineManager;
};
