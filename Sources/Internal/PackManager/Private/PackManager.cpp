#include "PackManager/PackManager.h"
#include "PackManager/Private/PackManagerImpl.h"
#include "FileSystem/FileSystem.h"

namespace DAVA
{
PackManager::PackManager()
{
    impl.reset(new PackManagerImpl());
};

PackManager::~PackManager() = default;

PackManager::IRequest::~IRequest() = default;

PackManager::ISync::~ISync() = default;

void PackManager::Initialize(const String& dbFileName_,
                             const FilePath& readOnlyPacksDir_,
                             const String& architecture_)
{
    impl->Initialize(dbFileName_, readOnlyPacksDir_, architecture_, this);
}

void PackManager::SyncWithServer(const String& urlToServerSuperpack, const FilePath& downloadPacksDir)
{
    impl->SyncWithServer(urlToServerSuperpack, downloadPacksDir);
}

PackManager::ISync& PackManager::GetISync()
{
    return *impl;
}

bool PackManager::IsRequestingEnabled() const
{
    return impl->IsProcessingEnabled();
}

void PackManager::EnableRequesting()
{
    impl->EnableProcessing();
}

void PackManager::DisableRequesting()
{
    impl->DisableProcessing();
}

void PackManager::Update()
{
    // first check if something downloading
    impl->Update();
}

const String& PackManager::FindPackName(const FilePath& relativePathInPack) const
{
    return impl->FindPackName(relativePathInPack);
}

const PackManager::Pack& PackManager::FindPack(const String& packName) const
{
    return impl->GetPack(packName);
}

const PackManager::Pack& PackManager::RequestPack(const String& packID)
{
    return impl->RequestPack(packID);
}

void PackManager::ChangeDownloadOrder(const String& packName, float newPriority)
{
    impl->ChangePackPriority(packName, newPriority);
}

const Vector<PackManager::Pack>& PackManager::GetPacks() const
{
    return impl->GetAllState();
}

void PackManager::DeletePack(const String& packID)
{
    impl->DeletePack(packID);
}

const FilePath& PackManager::GetLocalPacksDirectory() const
{
    return impl->GetLocalPacksDir();
}

const String& PackManager::GetSuperPackUrl() const
{
    return impl->GetSuperPackUrl();
}

} // end namespace DAVA