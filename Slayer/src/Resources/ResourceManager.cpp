#include "Resources/ResourceManager.h"

namespace Slayer
{
	ResourceManager* ResourceManager::instance = nullptr;


	std::future<GPULoadData> ResourceManager::LoadAssetsAsync(const std::string& assetPackPath)
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
						gpuLoadData.textures.push_back({ ta, record });
						continue;
					}
					case AssetType::SL_ASSET_TYPE_SHADER:
					{
						ShaderAsset sa = assetPack.GetAssetData<ShaderAsset>(id);
						gpuLoadData.shaders.push_back({ sa, record });
						continue;
					}
					case AssetType::SL_ASSET_TYPE_MATERIAL:
					{
						MaterialAsset ma = assetPack.GetAssetData<MaterialAsset>(id);
						Shared<Material> material = MakeShared<Material>();
						for (auto& texture : ma.textures)
						{
							material->SetTextures((TextureType)texture.type, texture.textureId);
						}
						m_assetStore.AddAsset(id, record.name, material);
						continue;
					}
					case AssetType::SL_ASSET_TYPE_MODEL:
					{
						ModelAsset ma = assetPack.GetAssetData<ModelAsset>(id);
						gpuLoadData.models.push_back({ ma, record });
						continue;
					}
					case AssetType::SL_ASSET_TYPE_SKELETAL_MODEL:
					{
						SkeletalModelAsset sma = assetPack.GetAssetData<SkeletalModelAsset>(id);
						gpuLoadData.skeletalModels.push_back({ sma, record });
						continue;
					}
					case AssetType::SL_ASSET_TYPE_ANIMATION:
					{
						AnimationAsset aa = assetPack.GetAssetData<AnimationAsset>(id);
						gpuLoadData.animations.push_back({ aa, record });
						continue;
					}
					default:
						break;
					}
				}

				return gpuLoadData;
			});
	}

	void ResourceManager::LoadGPUAssets(GPULoadData& gpuLoadData)
	{
		for (auto& [ta, record] : gpuLoadData.textures)
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

		for (auto& [sa, record] : gpuLoadData.shaders)
		{
			Shared<Shader> shader = Shader::LoadShader(sa.vsSource, sa.fsSource);
			m_assetStore.AddAsset(record.id, record.name, shader);
		}

		for (auto& [ma, record] : gpuLoadData.models)
		{
			Shared<Model> model = MakeShared<Model>();
			for (auto& meshDesc : ma.meshes)
			{
				Shared<Mesh> mesh = Mesh::Create((float*)meshDesc.vertices.data(), (uint32_t)meshDesc.vertices.size() * sizeof(float), (uint32_t*)meshDesc.indices.data(), (uint32_t)meshDesc.indices.size());
				model->AddMesh(mesh);
			}
			m_assetStore.AddAsset(record.id, record.name, model);
		}

		for (auto& [sma, record] : gpuLoadData.skeletalModels)
		{
			auto& mesh = sma.meshes[0];

			// Populate bone map.
			Dict<std::string, BoneInfo> bones;
			for (auto& bone : mesh.bones)
				bones[bone.name] = { bone.name, bone.id, bone.parentId, bone.transform };

			static_assert(sizeof(SkeletalVertex) == 64);

			Vector<SkeletalVertex> vertices(mesh.vertices.size());
			Copy(mesh.vertices.data(), vertices.data(), mesh.vertices.size() * sizeof(SkeletalVertex));

			Shared<SkeletalModel> skeletalModel = SkeletalModel::Create(vertices, mesh.indices, bones, mesh.globalInverseTransform);
			skeletalModel->AddSockets(sma.sockets);
			m_assetStore.AddAsset(record.id, record.name, skeletalModel);
		}

		for (auto& [aa, record] : gpuLoadData.animations)
		{
			Shared<Animation> animation = Animation::Create(aa.data, aa.times, aa.duration);
			m_assetStore.AddAsset(record.id, record.name, animation);
		}
	}


}