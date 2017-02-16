#ifndef __DAVAENGINE_AUTOTESTING_SYSTEM_H__
#define __DAVAENGINE_AUTOTESTING_SYSTEM_H__

#include "DAVAConfig.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "DAVAEngine.h"
#include "Base/Singleton.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/File.h"
#include "Time/DateTime.h"

#include "Autotesting/AutotestingSystemLua.h"

namespace DAVA
{
class Image;
class AutotestingSystemLuaDelegate;
class AutotestingSystemLua;
class AutotestingSystem : public Singleton<AutotestingSystem>
{
public:
    AutotestingSystem();
    ~AutotestingSystem();

    void OnAppStarted();
    void OnAppFinished();
    void OnTestSkipped();

    void Update(float32 timeElapsed);
    void Draw();

    void OnInit();
    inline bool IsInit()
    {
        return isInit;
    };

    void InitLua(AutotestingSystemLuaDelegate* _delegate);

    void RunTests();

    // Parameters from DB
    void FetchParametersFromDB();
    void FetchParametersFromIdYaml();
    void SetUpConnectionToDB();
    RefPtr<KeyedArchive> GetIdYamlOptions();

    void InitializeDevice();

    // Test organization
    void OnTestStart(const String& testName);
    void OnStepStart(const String& stepName);
    void OnStepFinished();
    void OnTestStarted();
    void OnError(const String& errorMessage = "");
    void ForceQuit(const String& logMessage = "");
    void OnTestsFinished();

    // helpers
    void OnInput(const UIEvent& input);

    inline Vector2 GetMousePosition()
    {
        return mouseMove.point;
    };
    bool FindTouch(int32 id, UIEvent& touch);
    bool IsTouchDown(int32 id);

    const String& GetScreenShotName();
    void MakeScreenShot();
    bool GetIsScreenShotSaving() const;
    void ClickSystemBack();
    void PressEscape();

    // DB Master-Helper relations

    String GetTestId()
    {
        return Format("Test%03d", testIndex);
    };
    String GetStepId()
    {
        return Format("Step%03d", stepIndex);
    };
    String GetLogId()
    {
        return Format("Message%03d", logIndex);
    };

    String GetCurrentTimeString();
    String GetCurrentTimeMsString();

    inline AutotestingSystemLua* GetLuaSystem()
    {
        return luaSystem;
    };

    bool ResolvePathToAutomation();
    FilePath GetPathTo(const String& path);

    void OnRecordClickControl(UIControl*);
    void OnRecordWaitControl(UIControl*);
    void OnRecordDoubleClickControl(UIControl*);
    void OnRecordSetText(UIControl*);
    void OnRecordCheckText(UIControl*);
    void OnRecordFastSelectControl(UIControl*);
    void StartRecording();
    void StopRecording();
    bool IsRecording()
    {
        return isRecording;
    }

    void OnRightMouseButton(UIEvent* e);

protected:
    void DrawTouches();
    void OnScreenShotInternal(Texture* texture);
    void OnWindowSizeChanged(Window*, Size2f windowSize, Size2f surfaceSize);

    void ResetScreenshotTexture(Size2i size);

    AutotestingSystemLua* luaSystem;
    //DB
    void ExitApp();

    //Recording
    const String GetControlHierarchy(UIControl*);
    const String WrapWithFunctionCall(const String&, const String&);
    void SaveLineToFile(const String&, const String&);

private:
    bool isScreenShotSaving = false;
    FilePath pathToAutomation;
    File* recordedActs;

public:
    static const String RecordScriptFileName;
    float32 startTime = 0.f;

    bool isInit;
    bool isRunning;
    bool needExitApp;
    float32 timeBeforeExit;

    String projectName;
    String groupName;
    String deviceName;
    String testsDate;
    String runId;
    int32 testIndex;
    int32 stepIndex;
    int32 logIndex;

    String testDescription;
    String testFileName;
    String testFilePath;

    String buildDate;
    String buildId;
    String branch;
    String framework;
    String branchRev;
    String frameworkRev;

    bool isDB;
    bool needClearGroupInDB;

    bool isMaster;
    int32 requestedHelpers;
    String masterId; // for communication
    String masterTask;
    int32 masterRunId;
    bool isRegistered;
    bool isWaiting;
    bool isInitMultiplayer;
    String multiplayerName;
    float32 waitTimeLeft;
    float32 waitCheckTimeLeft;

    Map<int32, UIEvent> touches;
    UIEvent mouseMove;

    String screenshotName;
    Texture* screenshotTexture = nullptr;
    rhi::HSyncObject screenshotSync;
    bool screenshotRequested = false;

    TrackedObject localTrackedObject;

    bool isRecording = false;
};

inline bool AutotestingSystem::GetIsScreenShotSaving() const
{
    return isScreenShotSaving;
}
};

#endif //__DAVAENGINE_AUTOTESTING__

#endif //__DAVAENGINE_AUTOTESTING_SYSTEM_H__
