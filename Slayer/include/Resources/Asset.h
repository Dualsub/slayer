#pragma once

#include "Slayer.h"
#include <random>

namespace Slayer
{
    using AssetID = uint64_t;

    enum AssetType : uint16_t
    {
        SL_ASSET_TYPE_NONE = 0,
        SL_ASSET_TYPE_TEXTURE = 1,
        SL_ASSET_TYPE_SHADER = 2,
        SL_ASSET_TYPE_MODEL = 3,
        SL_ASSET_TYPE_SKELETAL_MODEL = 4,
        SL_ASSET_TYPE_MATERIAL = 5,
        SL_ASSET_TYPE_ANIMATION = 6,
        SL_ASSET_TYPE_SOUND = 7,
        SL_ASSET_TYPE_FONT = 8,
        SL_ASSET_TYPE_PREFAB = 9,
        SL_ASSET_TYPE_SCENE = 10,
        SL_ASSET_TYPE_COMPUTE_SHADER = 11,
    };

    static AssetID GenerateAssetID()
    {
        // Generate a random ID
        static std::random_device rd;
        static std::mt19937_64 gen(rd());
        static std::uniform_int_distribution<AssetID> dis(0, std::numeric_limits<AssetID>::max());

        return dis(gen);
    }

    class Asset
    {
    private:
        AssetID m_id;
        AssetType m_type;
        std::string m_name;
    public:
        Asset();
        ~Asset();

        AssetID GetID() const { return m_id; };
        AssetType GetType() const { return m_type; };
        constexpr bool IsGPUResource() const { return m_type == SL_ASSET_TYPE_TEXTURE || m_type == SL_ASSET_TYPE_SHADER || m_type == SL_ASSET_TYPE_MODEL || m_type == SL_ASSET_TYPE_SKELETAL_MODEL || m_type == SL_ASSET_TYPE_MATERIAL || m_type == SL_ASSET_TYPE_ANIMATION; }
        const std::string& GetName() const { return m_name; }
        void* GetData() const;
    };

} // namespace Slayer