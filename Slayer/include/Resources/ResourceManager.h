#pragma once

#include "Core/Containers.h"

#include "Resources/Asset.h"
#include "Resources/AssetPack.h"
#include "Resources/AssetTypes.h"
#include "Resources/AssetStore.h"
#include "Rendering/Renderer/Texture.h"
#include "Rendering/Renderer/Shader.h"
#include "Rendering/Renderer/ComputeShader.h"
#include "Rendering/Renderer/Material.h"
#include "Rendering/Renderer/Model.h"
#include "Rendering/Renderer/SkeletalModel.h"
#include "Rendering/Animation/Animation.h"

#include <future>
#include <thread>

namespace Slayer
{
    // A struct that holds all the GPU resources that need to be loaded on the GPU
    struct GPULoadData
    {
        Vector<Tuple<TextureAsset, AssetRecord>> textures = {};
        Vector<Tuple<ShaderAsset, AssetRecord>> shaders = {};
        Vector<Tuple<ComputeShaderAsset, AssetRecord>> computeShaders = {};
        Vector<Tuple<ModelAsset, AssetRecord>> models = {};
        Vector<Tuple<SkeletalModelAsset, AssetRecord>> skeletalModels = {};
        Vector<Tuple<AnimationAsset, AssetRecord>> animations = {};

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

        std::future<GPULoadData> LoadAssetsAsync(const std::string& assetPackPath);
        void LoadGPUAssets(GPULoadData& gpuLoadData);

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