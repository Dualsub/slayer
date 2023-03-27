#include "Rendering/Renderer/Material.h"
#include "glad/glad.h"

namespace Slayer {

	Material::Material(const Map<TextureType, AssetID>& textures) :
		textures(textures)
	{
	}

	void Material::AttachTexture(const AssetID& texture, TextureType type)
	{
		textures.emplace(type, texture);
	}

	Shared<Material> Material::LoadMaterial(const Map<TextureType, AssetID>& textures)
	{
		return std::make_shared<Material>(textures);
	}

	void Material::SetTextures(TextureType type, AssetID texture)
	{
		textures[type] = texture;
	}

	std::string Material::GetMaterialTypeName(const TextureType& type)
	{
		std::string name;
		switch (type)
		{
		case Slayer::ALBEDO:
			name = "Albedo";
			break;
		case Slayer::NORMAL:
			name = "Normal";
			break;
		case Slayer::METALLIC:
			name = "Metallic";
			break;
		case Slayer::ROUGHNESS:
			name = "Roughness";
			break;
		case Slayer::AMBIENT:
			name = "Ambient Occlusion ";
			break;
		default:
			name = "None";
			break;
		}
		return name;
	}

}

