#include "AssetCache/CachePacket.h"
#include "FileSystem/StaticMemoryFile.h"
#include "Network/IChannel.h"

namespace DAVA
{
namespace AssetCache
{
const uint16 PACKET_HEADER = 0xACCA;
const uint8 PACKET_VERSION = 1;

Map<const uint8*, ScopedPtr<DynamicMemoryFile>> CachePacket::sendingPackets;

bool CachePacket::SendTo(Net::IChannel* channel)
{
    DVASSERT(channel);

    auto insertRes = sendingPackets.insert(std::make_pair(serializationBuffer->GetData(), serializationBuffer));
    DVASSERT(true == insertRes.second && "packet is already inserted");

    uint32 packetId = 0;
    channel->Send(serializationBuffer->GetData(), static_cast<size_t>(serializationBuffer->GetSize()), 0, &packetId);
    return (packetId != 0);
}

void CachePacket::PacketSent(const uint8* buffer, size_t length)
{
    DVASSERT(sendingPackets.empty() == false);

    auto found = sendingPackets.find(buffer);
    DVASSERT(found != sendingPackets.end() && "packet is not found in sending list");
    DVASSERT(found->second->GetSize() == length);
    sendingPackets.erase(found);
}

std::unique_ptr<CachePacket> CachePacket::Create(const uint8* rawdata, uint32 length)
{
    ScopedPtr<File> buffer(StaticMemoryFile::Create(const_cast<uint8*>(rawdata), length, File::OPEN | File::READ));

    CachePacketHeader header;
    if (buffer->Read(&header) != sizeof(header))
    {
        Logger::Error("[CachePacket::%s] Cannot read header: %s", __FUNCTION__);
        return nullptr;
    }

    if (header.headerID != PACKET_HEADER || header.version != PACKET_VERSION || header.packetType == PACKET_UNKNOWN)
    {
        Logger::Error("[CachePacket::%s] Wrong header: id(%d), version(%d), packet type(%d)", __FUNCTION__, header.headerID, header.version, header.packetType);
        return nullptr;
    }

    std::unique_ptr<CachePacket> packet = CachePacket::CreateByType(header.packetType);
    if (packet != nullptr)
    {
        bool loaded = packet->Load(buffer);
        if (!loaded)
        {
            Logger::Error("[CachePacket::%s] Cannot load packet(type: %d)", __FUNCTION__, header.packetType);
            packet.reset();
        }
    }

    return packet;
}

std::unique_ptr<CachePacket> CachePacket::CreateByType(ePacketID type)
{
    switch (type)
    {
    case PACKET_ADD_REQUEST:
        return std::unique_ptr<CachePacket>(new AddRequestPacket());
    case PACKET_ADD_RESPONSE:
        return std::unique_ptr<CachePacket>(new AddResponsePacket());
    case PACKET_GET_REQUEST:
        return std::unique_ptr<CachePacket>(new GetRequestPacket());
    case PACKET_GET_RESPONSE:
        return std::unique_ptr<CachePacket>(new GetResponsePacket());
    case PACKET_WARMING_UP_REQUEST:
        return std::unique_ptr<CachePacket>(new WarmupRequestPacket());
    default:
    {
        Logger::Error("[CachePacket::%s] Wrong packet type: %d", __FUNCTION__, type);
        break;
    }
    }

    return nullptr;
}

CachePacket::CachePacket(ePacketID _type, bool createBuffer)
    : type(_type)
    , serializationBuffer(nullptr)
{
    if (createBuffer)
    {
        serializationBuffer.reset(DynamicMemoryFile::Create(File::CREATE | File::WRITE));
    }
}

void CachePacket::WriteHeader(File* file) const
{
    CachePacketHeader header(PACKET_HEADER, PACKET_VERSION, type);
    file->Write(&header);
}

AddRequestPacket::AddRequestPacket(const CacheItemKey& _key, const CachedItemValue& _value)
    : CachePacket(PACKET_ADD_REQUEST, true)
{
    WriteHeader(serializationBuffer);

    serializationBuffer->Write(_key.data(), static_cast<uint32>(_key.size()));
    _value.Serialize(serializationBuffer);
}

bool AddRequestPacket::Load(File* file)
{
    const uint32 keySize = static_cast<uint32>(key.size());
    return ((file->Read(key.data(), keySize) == keySize) && value.Deserialize(file));
}

AddResponsePacket::AddResponsePacket(const CacheItemKey& _key, bool _added)
    : CachePacket(PACKET_ADD_RESPONSE, true)
{
    WriteHeader(serializationBuffer);

    serializationBuffer->Write(_key.data(), static_cast<uint32>(_key.size()));
    serializationBuffer->Write(&_added, sizeof(_added));
}

bool AddResponsePacket::Load(File* file)
{
    const uint32 keySize = static_cast<uint32>(key.size());
    return ((file->Read(key.data(), keySize) == keySize) && (file->Read(&added) == sizeof(added)));
}

GetRequestPacket::GetRequestPacket(const CacheItemKey& _key)
    : CachePacket(PACKET_GET_REQUEST, true)
{
    WriteHeader(serializationBuffer);

    serializationBuffer->Write(_key.data(), static_cast<uint32>(_key.size()));
}

bool GetRequestPacket::Load(File* file)
{
    const uint32 keySize = static_cast<uint32>(key.size());
    return (file->Read(key.data(), keySize) == keySize);
}

GetResponsePacket::GetResponsePacket(const CacheItemKey& _key, const CachedItemValue& _value)
    : CachePacket(PACKET_GET_RESPONSE, true)
{
    WriteHeader(serializationBuffer);

    serializationBuffer->Write(_key.data(), static_cast<uint32>(_key.size()));
    _value.Serialize(serializationBuffer);
}

bool GetResponsePacket::Load(File* file)
{
    const uint32 keySize = static_cast<uint32>(key.size());
    return ((file->Read(key.data(), keySize) == keySize) && value.Deserialize(file));
}

WarmupRequestPacket::WarmupRequestPacket(const CacheItemKey& _key)
    : CachePacket(PACKET_WARMING_UP_REQUEST, true)
{
    WriteHeader(serializationBuffer);

    serializationBuffer->Write(_key.data(), static_cast<uint32>(_key.size()));
}

bool WarmupRequestPacket::Load(File* file)
{
    const uint32 keySize = static_cast<uint32>(key.size());
    return (file->Read(key.data(), keySize) == keySize);
}

} //AssetCache
} //DAVA
