#if !defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Infrastructure/BaseScreen.h"
#include <FileSystem/FilePath.h>
#include <PackManager/PackManager.h>

class PackManagerTest : public BaseScreen, DAVA::UITextFieldDelegate
{
public:
    PackManagerTest();

private:
    void TextFieldOnTextChanged(DAVA::UITextField* textField, const DAVA::WideString& newText, const DAVA::WideString& /*oldText*/) override;
    void UpdateDescription();

    void LoadResources() override;
    void UnloadResources() override;

    void OnStartDownloadClicked(DAVA::BaseObject* sender, void* data, void* callerData);
    void OnStartStopLocalServerClicked(DAVA::BaseObject* sender, void* data, void* callerData);

    void OnPackStateChange(const DAVA::PackManager::Pack& pack, DAVA::PackManager::Pack::Change change);

    DAVA::FilePath sqliteDbFile = "~res:/TestData/PackManagerTest/packs/testbed_{gpu}.db";
    DAVA::FilePath folderWithDownloadedPacks = "~doc:/PackManagerTest/packs/";
    // TODO quick and dirty way to test download on all platforms, in future replace with local http server
    DAVA::String urlPacksCommon = "http://by1-builddlc-01.corp.wargaming.local/DLC_Blitz/packs/common/"; //"http://127.0.0.1:2424/packs/common/";
    DAVA::String urlPacksGpu = "http://by1-builddlc-01.corp.wargaming.local/DLC_Blitz/packs/{gpu}/"; //"http://127.0.0.1:2424/packs/{gpu}/";
    DAVA::String gpuName;

    DAVA::UIStaticText* packNameLoading = nullptr;
    DAVA::UIButton* startLoadingButton = nullptr;

    DAVA::UIButton* startServerButton = nullptr;
    DAVA::UIButton* stopServerButton = nullptr;

    DAVA::UIControl* progressBar = nullptr;
    DAVA::UITextField* packInput = nullptr;
    DAVA::UIControl* redControl = nullptr;
    DAVA::UIControl* greenControl = nullptr;
    DAVA::UIStaticText* description = nullptr;
    DAVA::UITextField* url = nullptr;
};

#endif // !__DAVAENGINE_COREV2__
