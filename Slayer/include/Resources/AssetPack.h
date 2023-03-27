#pragma once

#include "Slayer.h"
#include "Asset.h"

#include <fstream>

#define SL_ASSET_PACK_VERSION 1
#define SL_ASSET_PACK_MAGIC "SLPCK"

namespace Slayer
{
    struct AssetPackHeader
    {
        char magic[6];
        uint32_t version;
        uint32_t numAssets;

        void Read(std::ifstream& stream)
        {
            stream.read(magic, 6);
            stream.read((char*)&version, sizeof(uint32_t));
            stream.read((char*)&numAssets, sizeof(uint32_t));
        }
    };

    struct AssetHeader
    {
        AssetID id;
        AssetType type;
        uint32_t nameLength;
        uint32_t dataLength;

        void Read(std::ifstream& stream)
        {
            stream.read((char*)&id, sizeof(AssetID));
            stream.read((char*)&type, sizeof(AssetType));
            stream.read((char*)&nameLength, sizeof(uint32_t));
            stream.read((char*)&dataLength, sizeof(uint32_t));
        }

        uint32_t GetSize() const
        {
            return sizeof(AssetID) + sizeof(AssetType) + sizeof(uint32_t) + sizeof(uint32_t);
        }
    };

    struct AssetRecord
    {
        AssetID id;
        AssetType type;
        std::string name;
        uint32_t dataLength;
        uint32_t dataIndex;

        void Read(std::ifstream& stream)
        {
            stream.read((char*)&id, sizeof(AssetID));
            stream.read((char*)&type, sizeof(AssetType));
            stream.read((char*)&dataLength, sizeof(uint32_t));
            stream.read((char*)&dataIndex, sizeof(uint32_t));
        }
    };

    class AssetPack
    {
    private:
        bool m_isLoaded;
        std::vector<char> m_data;
        Dict<std::string, AssetID> m_assetNames;
        Dict<AssetID, AssetRecord> m_assets;

    public:
        AssetPack() = default;
        ~AssetPack() = default;

        void Load(const std::string& path);
        void Save(const std::string& path);

        bool IsLoaded() const { return m_isLoaded; }
        const Dict<AssetID, AssetRecord>& GetAssets() const { return m_assets; }

        template<typename T>
        T GetAssetData(const AssetID& id) const
        {
            SL_ASSERT(m_assets.find(id) != m_assets.end() && "Asset not found!");

            const AssetRecord& record = m_assets.at(id);
            T data;
            data.Read((char*)(m_data.data() + record.dataIndex));
            return data;
        }
    };

}