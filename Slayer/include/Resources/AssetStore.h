#pragma once

// Runtime storage for assets

#include "Core/Core.h"
#include "Core/Containers.h"
#include "Resources/Asset.h"

#include <typeinfo>

namespace Slayer
{
    using AssetTypeHash = size_t;

    template<typename T>
    constexpr AssetTypeHash HashAssetType()
    {
        return typeid(T).hash_code();
    }

    class AssetArrayBase
    {
    public:
        virtual ~AssetArrayBase() = default;
    };

    template<typename T>
    class AssetArray: public AssetArrayBase
    {
    private:
        Dict<AssetID, Shared<T>> m_data;
    public:
        AssetArray() = default;
        ~AssetArray() = default;

        void AddAsset(const AssetID& id, Shared<T> asset)
        {
            m_data[id] = asset;
        }

        Shared<T> GetAsset(const AssetID& id)
        {
            return m_data[id];
        }
    };

    class AssetStore
    {
    private:
        Dict<std::string, AssetID> m_namesToIDs;
        Dict<AssetTypeHash, Unique<AssetArrayBase>> m_assets;
    public:
        AssetStore() = default;
        ~AssetStore() = default;

        template<typename T>
        void RegisterAssetType()
        {
            SL_ASSERT(m_assets.find(HashAssetType<T>()) == m_assets.end() && "Asset type already registered");
            m_assets[HashAssetType<T>()] = MakeUnique<AssetArray<T>>();
        }

        template<typename T>
        void AddAsset(const AssetID& id, const std::string& name, Shared<T> asset)
        {
            if (m_assets.find(HashAssetType<T>()) == m_assets.end())
                RegisterAssetType<T>();

            AssetArray<T>* array = (AssetArray<T>*)m_assets[HashAssetType<T>()].get();
            array->AddAsset(id, asset);
            
            m_namesToIDs[name] = id;
        }

        template<typename T>
        Shared<T> GetAsset(const AssetID& id)
        {
            SL_ASSERT(m_assets.find(HashAssetType<T>()) != m_assets.end() && "Asset type not registered");
            return static_cast<AssetArray<T>*>(m_assets[HashAssetType<T>()].get())->GetAsset(id);
        }

        template<typename T>
        Shared<T> GetAsset(const std::string& assetName)
        {
            SL_ASSERT(m_assets.find(HashAssetType<T>()) != m_assets.end() && m_namesToIDs.find(assetName) != m_namesToIDs.end() && "Asset type not registered or asset not found");
            return static_cast<AssetArray<T>*>(m_assets[HashAssetType<T>()].get())->GetAsset(m_namesToIDs[assetName]);
        }

        AssetID GetAssetID(const std::string& assetName)
        {
            SL_ASSERT(m_namesToIDs.find(assetName) != m_namesToIDs.end() && "Asset not found");
            return m_namesToIDs[assetName];
        }

    };
}

