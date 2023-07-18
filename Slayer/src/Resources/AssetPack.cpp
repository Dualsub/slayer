
#include "Slayer.h"
#include "Resources/AssetPack.h"
#include <iostream>
#include <fstream>
#include <filesystem>

namespace Slayer
{
    void AssetPack::Save(const std::string& path)
    {
    }

    void AssetPack::Load(const std::string& path)
    {
        Log::Info("Loading asset pack: " + std::filesystem::absolute(path).string());
        // Attach an input stream to the wanted file
        std::ifstream inputStream(path, std::ios::binary);
        // Check stream status
        if (!inputStream.is_open())
            std::cerr << path << std::endl;
        SL_ASSERT(inputStream.is_open() && "Failed to open asset pack!");

        // Read the header
        AssetPackHeader header;
        header.Read(inputStream);

        std::cout << "Loaded asset pack: " << path << std::endl;
        std::cout << "Asset pack magic: " << header.magic << std::endl;
        std::cout << "Asset pack version: " << header.version << std::endl;
        std::cout << "Number of assets: " << header.numAssets << std::endl;

        // Check the header
        SL_ASSERT(std::string(header.magic) == SL_ASSET_PACK_MAGIC);

        // Check the version
        SL_ASSERT(header.version == SL_ASSET_PACK_VERSION);

        m_data = std::vector<char>();
        uint32_t dataIndex = 0;
        // Read assets
        for (uint32_t i = 0; i < header.numAssets; i++)
        {
            AssetHeader assetHeader;
            assetHeader.Read(inputStream);
            AssetRecord record;
            record.id = assetHeader.id;
            record.type = assetHeader.type;
            record.name = assetHeader.name;

            std::cout << "Asset name: " << record.name << std::endl;
            std::cout << "\tData Length: " << assetHeader.dataLength << std::endl;

            record.dataIndex = dataIndex;
            dataIndex += assetHeader.dataLength;
            record.dataLength = assetHeader.dataLength;

            m_assets[record.id] = record;
            m_assetNames[record.name] = record.id;

            // Add the data to the data buffer
            std::vector<char> data;
            data.resize(assetHeader.dataLength);
            inputStream.read(data.data(), assetHeader.dataLength);
            m_data.insert(m_data.end(), data.begin(), data.end());


        }
    }

}
