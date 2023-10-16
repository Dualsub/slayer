#pragma once

#include "Core/Core.h"
#include "Core/Containers.h"
#include "Resources/Asset.h"

namespace Slayer {

	enum TextureType : uint8_t
	{
		ALBEDO = 4,
		NORMAL = 5,
		METALLIC = 6,
		ROUGHNESS = 7,
		AMBIENT = 8
	};

	enum UniformType
	{
		UNIFORM_FLOAT,
		UNIFORM_INT,
		UNIFORM_VEC2,
		UNIFORM_VEC3,
		UNIFORM_VEC4,
		UNIFORM_MAT3,
		UNIFORM_MAT4
	};

	class Material
	{
	private:
		AssetID overrideShader;
		// Only handles one texture per type for now.
		Map<TextureType, AssetID> textures = {
			{ TextureType::ALBEDO,		SL_INVALID_ASSET_ID },
			{ TextureType::NORMAL,		SL_INVALID_ASSET_ID },
			{ TextureType::METALLIC,	SL_INVALID_ASSET_ID },
			{ TextureType::ROUGHNESS,	SL_INVALID_ASSET_ID },
			{ TextureType::AMBIENT,		SL_INVALID_ASSET_ID },
		};
	public:
		AssetID assetID;

		Material() = default;
		Material(const Map<TextureType, AssetID>& textures);
		static std::string GetMaterialTypeName(const TextureType& type);
		static Shared<Material> LoadMaterial(const Map<TextureType, AssetID>& textures);
		static Shared<Material> LoadMaterial() { return std::make_shared<Material>(); }
		void SetTextures(TextureType type, AssetID texture);
		void SetOverrideShader(AssetID inShader) { overrideShader = inShader; }
		Map<TextureType, AssetID>& GetTextures() { return textures; }
		void AttachTexture(const AssetID& texture, TextureType type);
		inline const AssetID& GetOverrideShader() { return overrideShader; }
	};

}