#include "AssetsManagerAndroid.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/File.h"
#include "FileSystem/Private/ZipArchive.h"
#include "Base/RefPtr.h"

namespace DAVA
{
AssetsManagerAndroid::AssetsManagerAndroid() = default;

AssetsManagerAndroid::~AssetsManagerAndroid() = default;

static const String assetsDirectory = "assets/Data/";

void AssetsManagerAndroid::Init(const String& apkFileName)
{
    if (apk)
    {
        throw std::runtime_error("[AssetsManager::Init] should be initialized only once.");
    }

    FilePath apkPath(apkFileName);
    RefPtr<File> file(File::Create(apkPath, File::OPEN | File::READ));
    if (!file)
    {
        throw std::runtime_error("[AssetsManager::Init] can't open: " + apkFileName);
    }

    apk.reset(new ZipArchive(file, apkPath));
}

bool AssetsManagerAndroid::HasDirectory(const String& relativeDirName) const
{
    String nameInApk = assetsDirectory + relativeDirName;
    const Vector<ResourceArchive::FileInfo>& files = apk->GetFilesInfo();
    for (const ResourceArchive::FileInfo& info : files)
    {
        if (info.relativeFilePath.find(nameInApk) == 0)
        {
            return true;
        }
    }
    return false;
}

bool AssetsManagerAndroid::HasFile(const String& relativeFilePath) const
{
    return apk->HasFile(assetsDirectory + relativeFilePath);
}

bool AssetsManagerAndroid::LoadFile(const String& relativeFilePath, Vector<uint8>& output) const
{
    return apk->LoadFile(assetsDirectory + relativeFilePath, output);
}

bool AssetsManagerAndroid::ListDirectory(const String& relativeDirName, Vector<ResourceArchive::FileInfo>& names) const
{
    names.clear();

    Set<String> addedfilesAndDirs;
    const Vector<ResourceArchive::FileInfo>& files = apk->GetFilesInfo();
    for (const ResourceArchive::FileInfo& info : files)
    {
        String assetPath = assetsDirectory + relativeDirName;
        if (info.relativeFilePath.find(assetPath) == 0)
        {
            size_t nextDirSlash = info.relativeFilePath.find('/', assetPath.size());
            if (String::npos == nextDirSlash)
            {
                // add file
                addedfilesAndDirs.insert(info.relativeFilePath.substr(assetsDirectory.size()));
                names.push_back(info);
            }
            else
            {
                // add directory (without sub directories)
                String dir = info.relativeFilePath.substr(assetsDirectory.size(), nextDirSlash - assetsDirectory.size() + 1);
                if (addedfilesAndDirs.find(dir) == end(addedfilesAndDirs))
                {
                    ResourceArchive::FileInfo fi = info;
                    fi.relativeFilePath = dir;
                    fi.compressedCrc32 = 0;
                    fi.compressedSize = 0;
                    fi.originalCrc32 = 0;
                    fi.originalSize = 0;
                    names.push_back(fi);
                    addedfilesAndDirs.insert(dir);
                }
            }
        }
    }

    return !names.empty();
}

} // DAVA namespace
