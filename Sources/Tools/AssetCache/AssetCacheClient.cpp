#include "AssetCacheClient.h"

#include "FileSystem/FileSystem.h"
#include "Platform/SystemTimer.h"
#include "Concurrency/LockGuard.h"
#include "Concurrency/Thread.h"
#include "Job/JobManager.h"

namespace DAVA
{
AssetCacheClient::AssetCacheClient(bool emulateNetworkLoop_)
    : isActive(false)
    , isJobStarted(false)
    , emulateNetworkLoop(emulateNetworkLoop_)
{
    DVASSERT(JobManager::Instance() != nullptr);

    client.AddListener(this);
}

AssetCacheClient::~AssetCacheClient()
{
    client.RemoveListener(this);

    DVASSERT(isActive == false);
    DVASSERT(isJobStarted == false);
}

AssetCache::Error AssetCacheClient::ConnectSynchronously(const ConnectionParams& connectionParams)
{
    timeoutms = connectionParams.timeoutms;

    isActive = true;
    if (emulateNetworkLoop)
    {
        JobManager::Instance()->CreateWorkerJob(MakeFunction(this, &AssetCacheClient::ProcessNetwork));
    }

    bool connectCalled = client.Connect(connectionParams.ip, connectionParams.port);
    if (!connectCalled)
    {
        isActive = false;
        return AssetCache::Error::ADDRESS_RESOLVER_FAILED;
    }

    uint64 startTime = SystemTimer::Instance()->AbsoluteMS();
    while (client.ChannelIsOpened() == false)
    {
        uint64 deltaTime = SystemTimer::Instance()->AbsoluteMS() - startTime;
        if (((timeoutms > 0) && (deltaTime > timeoutms)) && (client.ChannelIsOpened() == false))
        {
            Logger::Error("[AssetCacheClient::%s] connection to %s:%hu refused by timeout (%lld ms)", __FUNCTION__, connectionParams.ip.c_str(), connectionParams.port, connectionParams.timeoutms);
            isActive = false;
            return AssetCache::Error::OPERATION_TIMEOUT;
        }
    }

    return AssetCache::Error::NO_ERRORS;
}

void AssetCacheClient::Disconnect()
{
    isActive = false;
    client.Disconnect();

    while (isJobStarted)
    {
        //wait for finishing of networking
    }
}

AssetCache::Error AssetCacheClient::AddToCacheSynchronously(const AssetCache::CacheItemKey& key, const AssetCache::CachedItemValue& value)
{
    {
        LockGuard<Mutex> guard(requestLocker);
        request = Request(key, nullptr, AssetCache::PACKET_ADD_REQUEST);
    }

    AssetCache::Error resultCode = AssetCache::Error::CANNOT_SEND_REQUEST_ADD;

    bool requestSent = client.AddToCache(key, value);
    if (requestSent)
    {
        resultCode = WaitRequest();
    }

    {
        LockGuard<Mutex> guard(requestLocker);
        request.Reset();
    }

    return resultCode;
}

AssetCache::Error AssetCacheClient::RequestFromCacheSynchronously(const AssetCache::CacheItemKey& key, AssetCache::CachedItemValue* value)
{
    DVASSERT(value != nullptr);

    {
        LockGuard<Mutex> guard(requestLocker);
        request = Request(key, value, AssetCache::PACKET_GET_REQUEST);
    }

    AssetCache::Error resultCode = AssetCache::Error::CANNOT_SEND_REQUEST_GET;

    bool requestSent = client.RequestFromCache(key);
    if (requestSent)
    {
        resultCode = WaitRequest();
    }

    {
        LockGuard<Mutex> guard(requestLocker);
        request.Reset();
    }

    return resultCode;
}

AssetCache::Error AssetCacheClient::WaitRequest()
{
    uint64 startTime = SystemTimer::Instance()->AbsoluteMS();

    Request currentRequest;
    {
        LockGuard<Mutex> guard(requestLocker);
        currentRequest = request;
    }

    while (currentRequest.recieved == false)
    {
        {
            LockGuard<Mutex> guard(requestLocker);
            currentRequest = request;
        }

        auto deltaTime = SystemTimer::Instance()->AbsoluteMS() - startTime;
        if (((timeoutms > 0) && (deltaTime > timeoutms)) && (currentRequest.recieved == false) && (currentRequest.processingRequest == false))
        {
            return AssetCache::Error::OPERATION_TIMEOUT;
        }
    }

    if (currentRequest.result == AssetCache::Error::NO_ERRORS)
    {
        while (currentRequest.processingRequest)
        {
            LockGuard<Mutex> guard(requestLocker);
            currentRequest = request;
        }
    }

    return currentRequest.result;
}

void AssetCacheClient::OnAddedToCache(const AssetCache::CacheItemKey& key, bool added)
{
    LockGuard<Mutex> guard(requestLocker);
    if (request.requestID == AssetCache::PACKET_ADD_REQUEST && request.key == key)
    {
        request.result = (added) ? AssetCache::Error::NO_ERRORS : AssetCache::Error::SERVER_ERROR;
        request.recieved = true;
        request.processingRequest = false;
    }
    else
    {
        //skip this request, because it was canceled by timeout
    }
}

void AssetCacheClient::OnReceivedFromCache(const AssetCache::CacheItemKey& key, const AssetCache::CachedItemValue& value)
{
    Request currentRequest;
    {
        LockGuard<Mutex> guard(requestLocker);
        currentRequest = request;
    }

    if (currentRequest.requestID == AssetCache::PACKET_GET_REQUEST && currentRequest.key == key)
    {
        auto DumpInfo = [](const AssetCache::CacheItemKey& key, const AssetCache::CachedItemValue& value)
        {
            const AssetCache::CachedItemValue::Description& description = value.GetDescription();
            Logger::Info("[%s] %s - %s", description.creationDate.c_str(), description.machineName.c_str(), AssetCache::KeyToString(key).c_str());
            Logger::FrameworkDebug("[AssetCacheClient::OnReceivedFromCache] addingChain(%s), receivingChain(%s), comment(%s)", description.addingChain.c_str(), description.receivingChain.c_str(), description.comment.c_str());
        };

        if (value.IsEmpty())
        {
            LockGuard<Mutex> guard(requestLocker);
            request.result = AssetCache::Error::NOT_FOUND_ON_SERVER;
            request.recieved = true;
            request.processingRequest = false;
        }
        else if (value.IsValid() == false)
        {
            LockGuard<Mutex> guard(requestLocker);
            request.result = AssetCache::Error::CORRUPTED_DATA;
            request.recieved = true;
            request.processingRequest = false;

            DumpInfo(key, value);
        }
        else
        {
            { // mark request as recieved and processed
                LockGuard<Mutex> guard(requestLocker);
                request.result = AssetCache::Error::NO_ERRORS;
                request.recieved = true;
                request.processingRequest = true;

                DVASSERT_MSG(request.value != nullptr, "Request object that waits for response of data, should have valid pointer to AssetCacheValue");
                *(request.value) = value;

                DumpInfo(key, value);
                request.processingRequest = false;
            }
        }
    }
    else
    {
        //skip this request, because it was canceled by timeout
    }
}

void AssetCacheClient::ProcessNetwork()
{
    isJobStarted = true;

    while (isActive)
    {
        Net::NetCore::Instance()->Poll();
    }

    isJobStarted = false;
}

void AssetCacheClient::OnAssetClientStateChanged()
{
    if (client.ChannelIsOpened() == false)
    {
        isActive = false;
    }
}

bool AssetCacheClient::IsConnected() const
{
    return client.ChannelIsOpened();
}

} //END of DAVA
