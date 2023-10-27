#pragma once

#include "Core/Core.h"
#include "glad/glad.h"
#include "Resources/Asset.h"

namespace Slayer {

	enum TextureTarget : uint32_t
	{
		TYPE_NONE,
		TEXTURE_2D = GL_TEXTURE_2D,
		TEXTURE_CUBE_MAP = GL_TEXTURE_CUBE_MAP
	};

	class Texture
	{
	private:
		unsigned int textureID;
		int slotOffset;
		TextureTarget target;

		static Texture LoadTextureData(const std::string& filePath, int slotOffset, TextureTarget target);
	public:
		static uint64_t GPU_TEXTURE_MEM_ALLOCATED;

		AssetID assetID;

		Texture(unsigned int textureID, int slotOffset = 0, TextureTarget target = TextureTarget::TEXTURE_2D);
		Texture();
		~Texture();

		void Bind();
		void Bind(int slotOffset);
		void Unbind();
		void Dispose();

		inline unsigned int GetTextureID() { return textureID; }
		inline TextureTarget GetTarget() { return target; }
		inline int GetSlot() { return slotOffset; }
		inline void SetSlot(int slot) { slotOffset = slot; }

		static Shared<Texture> CreateBuffer(uint32_t width, uint32_t height, uint32_t numChannels, uint32_t unit, uint32_t slotOffset = 0, TextureTarget target = TextureTarget::TEXTURE_2D);
		static Shared<Texture> LoadTexture(const uint8_t* data, uint32_t width, uint32_t height, uint32_t numChannels, uint32_t slotOffset = 0, TextureTarget target = TextureTarget::TEXTURE_2D);
		static Shared<Texture> LoadTexture(const uint8_t* data, uint32_t length, uint32_t slotOffset = 0, TextureTarget target = TextureTarget::TEXTURE_2D);
		static Shared<Texture> LoadTextureHDR(const float* data, uint32_t width, uint32_t height, uint32_t numChannels);
		static void BindTexture(unsigned int textureID, int slot = 0, TextureTarget target = TextureTarget::TEXTURE_2D);
	};
}