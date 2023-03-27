#include "DefaultLibrary.h"
#include "Core/Engine.h"


namespace Slayer {

	void DefaultRenderingLibrary::Load(const std::string& filePath)
	{
		DefaultRenderingLibraryDescription desc;
		YamlDeserializer deserializer;
		deserializer.Serialize(desc, filePath);

		const std::string& engineAssetsDirectory = Engine::Get()->GetEngineAssetsDirectory();
		
		defaultStaticShader = Shader::LoadShaderFromFiles(
			engineAssetsDirectory + desc.defaultStaticShaderVS, 
			engineAssetsDirectory + desc.defaultStaticShaderFS
		);
		defaultSkeletalShader = Shader::LoadShaderFromFiles(
			engineAssetsDirectory + desc.defaultSkeletalShaderVS, 
			engineAssetsDirectory + desc.defaultSkeletalShaderFS
		);
		
		defaultTextures.insert({ TextureType::ALBEDO, Texture::LoadTexture(engineAssetsDirectory + desc.defaultAlbedoPath) });
		defaultTextures[TextureType::ALBEDO]->SetSlot(TextureType::ALBEDO);
		defaultTextures.insert({ TextureType::NORMAL, Texture::LoadTexture(engineAssetsDirectory + desc.defaultNormalsPath) });
		defaultTextures[TextureType::NORMAL]->SetSlot(TextureType::NORMAL);
		defaultTextures.insert({ TextureType::AMBIENT, Texture::LoadTexture(engineAssetsDirectory + desc.defaultAmbientPath) });
		defaultTextures[TextureType::AMBIENT]->SetSlot(TextureType::AMBIENT);
		defaultTextures.insert({ TextureType::METALLIC, Texture::LoadTexture(engineAssetsDirectory + desc.defaultMetallicPath) });
		defaultTextures[TextureType::METALLIC]->SetSlot(TextureType::METALLIC);
		defaultTextures.insert({ TextureType::ROUGHNESS, Texture::LoadTexture(engineAssetsDirectory + desc.defaultRoughnessPath) });
		defaultTextures[TextureType::ROUGHNESS]->SetSlot(TextureType::ROUGHNESS);
	}

	void DefaultRenderingLibrary::CleanUp()
	{
		for (const auto& [type, texture] : defaultTextures)
		{
			if (defaultTextures.find(type) != defaultTextures.end())
			{
				defaultTextures[type]->Dispose();
				defaultTextures[type].reset();
			}
		}
		defaultTextures.clear();

		defaultStaticShader->Dispose();
		defaultStaticShader.reset();
		defaultSkeletalShader->Dispose();
		defaultSkeletalShader.reset();
	}

}