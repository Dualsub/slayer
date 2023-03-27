#pragma once

#include "Core/Containers.h"

#include "Resources/Asset.h"
#include "Resources/AssetPack.h"
#include "Resources/AssetTypes.h"
#include "Resources/AssetStore.h"
#include "Rendering/Renderer/Texture.h"
#include "Rendering/Renderer/Shader.h"
#include "Rendering/Renderer/Material.h"
#include "Rendering/Renderer/Model.h"
// #include "Rendering/Renderer/SkeletalModel.h"

#include <future>
#include <thread>

namespace Slayer
{
    // A struct that holds all the GPU resources that need to be loaded on the GPU
    struct GPULoadData
    {
        Vector<Tuple<TextureAsset, AssetRecord>> textures = {};
        Vector<Tuple<ShaderAsset, AssetRecord>> shaders = {};
        Vector<Tuple<ModelAsset, AssetRecord>> models = {};

        GPULoadData() = default;
        ~GPULoadData() = default;
    };

    class ResourceManager
    {
    public:
        ResourceManager() = default;
        ~ResourceManager() = default;

        static ResourceManager* Get() { return instance; }
        static void Initialize() { instance = new ResourceManager(); }
        static void Shutdown() { delete instance; }

        void LoadGPUAssets(GPULoadData& gpuLoadData)
        {
            for (auto& [ta, record]: gpuLoadData.textures)
            {
                if (ta.target == uint32_t(0x8513)) // HDR
                {
                    Shared<Texture> texture = Texture::LoadTextureHDR((const float*)ta.data.data(), ta.width, ta.height, ta.channels);
                    m_assetStore.AddAsset(record.id, record.name, texture);
                }
                else if (ta.target == uint32_t(0x0DE1)) // 2D
                {
                    Shared<Texture> texture = Texture::LoadTexture(ta.data.data(), ta.width, ta.height, ta.channels, (TextureTarget)ta.target);
                    m_assetStore.AddAsset(record.id, record.name, texture);
                }
                else
                {
                    SL_ASSERT(false && "Unknown texture target");
                }
            }

            for (auto& [sa, record]: gpuLoadData.shaders)
            {
                Shared<Shader> shader = Shader::LoadShader(sa.vsSource, sa.fsSource);
                m_assetStore.AddAsset(record.id, record.name, shader);
            }

            for (auto& [ma, record]: gpuLoadData.models)
            {
                Shared<Model> model = MakeShared<Model>();
                for (auto& meshDesc: ma.meshes)
                {
                    Shared<Mesh> mesh = Mesh::Create((float*)meshDesc.vertices.data(), (uint32_t)meshDesc.vertices.size() * sizeof(float), (uint32_t*)meshDesc.indices.data(), (uint32_t)meshDesc.indices.size());
                    model->AddMesh(mesh);
                }
                m_assetStore.AddAsset(record.id, record.name, model);
            }
        }

        std::future<GPULoadData> LoadAssetsAsync(const std::string& assetPackPath)
        {
            return std::async(std::launch::async, [this, assetPackPath]()
            {
                SL_ASSERT(this && "This pointer not valid!");
                AssetPack assetPack;
                assetPack.Load(assetPackPath);

                GPULoadData gpuLoadData;

                for (const auto& [id, record] : assetPack.GetAssets())
                {
                    switch (record.type)
                    {
                    case AssetType::SL_ASSET_TYPE_TEXTURE:
                    {
                        TextureAsset ta = assetPack.GetAssetData<TextureAsset>(id);
                        gpuLoadData.textures.push_back({ta, record});
                        continue;
                    }
                    case AssetType::SL_ASSET_TYPE_SHADER:
                    {
                        ShaderAsset sa = assetPack.GetAssetData<ShaderAsset>(id);
                        gpuLoadData.shaders.push_back({sa, record});
                        continue;
                    }
                    case AssetType::SL_ASSET_TYPE_MATERIAL:
                    {
                        MaterialAsset ma = assetPack.GetAssetData<MaterialAsset>(id);
                        Shared<Material> material = MakeShared<Material>();
                        for (auto& texture: ma.textures)
                        {
                            material->SetTextures((TextureType)texture.type, texture.textureId);
                        }
                        m_assetStore.AddAsset(id, record.name, material);
                        continue;
                    }
                    case AssetType::SL_ASSET_TYPE_MODEL:
                    {
                        ModelAsset ma = assetPack.GetAssetData<ModelAsset>(id);
                        gpuLoadData.models.push_back({ma, record});
                        continue;
                    }
                    default:
                        break;
                    }
                }

                return gpuLoadData;
            });
        }

        template<typename T>
        Shared<T> GetAsset(const AssetID& assetId)
        {
            return m_assetStore.GetAsset<T>(assetId);
        }

        template<typename T>
        Shared<T> GetAsset(const std::string& assetName)
        {
            return m_assetStore.GetAsset<T>(assetName);
        }

        AssetID GetAssetID(const std::string& assetName)
        {
            return m_assetStore.GetAssetID(assetName);
        }

    private:
        static ResourceManager* instance;
        AssetStore m_assetStore;
    };
}