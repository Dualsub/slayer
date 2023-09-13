#include "Core/Log.h"
#include "Rendering/Renderer/Texture.h"

namespace Slayer {

	uint64_t Texture::GPU_TEXTURE_MEM_ALLOCATED;

	Texture::Texture(unsigned int textureID, int slotOffset, TextureTarget target)
		: textureID(textureID), slotOffset(slotOffset), target(target)
	{
	}

	Texture::Texture()
	{
		textureID = 0;
		slotOffset = 0;
		target = TextureTarget::TEXTURE_2D;
	}

	Texture::~Texture()
	{
		glDeleteTextures(1, &textureID);
	}

	void Texture::Bind()
	{
		glActiveTexture(GL_TEXTURE0 + slotOffset);
		glBindTexture(target, textureID);
	}

	void Texture::Bind(int slotOffset)
	{
		glActiveTexture(GL_TEXTURE0 + slotOffset);
		glBindTexture(target, textureID);
	}

	void Texture::Unbind()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Texture::Dispose()
	{
		glDeleteTextures(1, &textureID);
	}

	Shared<Texture> Texture::LoadTexture(const uint8_t* data, uint32_t width, uint32_t height, uint32_t numChannels, uint32_t slotOffset, TextureTarget target)
	{
		unsigned int textureID;
		glGenTextures(1, &textureID);

		if (data)
		{
			GLenum format = GL_RGB;
			switch (numChannels)
			{
			case 1: format = GL_RED; break;
			case 3: format = GL_RGB; break;
			case 4: format = GL_RGBA; break;
			}
			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			Slayer::Log::Error("Failed to load texture!");
		}

		GPU_TEXTURE_MEM_ALLOCATED += (width * height * numChannels * sizeof(uint8_t));

		return MakeShared<Texture>(textureID, slotOffset, target);
	}

	Shared<Texture> Texture::CreateBuffer(uint32_t width, uint32_t height, uint32_t numChannels, uint32_t unit, uint32_t slotOffset, TextureTarget target)
	{
		unsigned int textureID;
		glGenTextures(1, &textureID);
		glActiveTexture(GL_TEXTURE0 + slotOffset);
		GLenum format = GL_RGB;
		switch (numChannels)
		{
		case 1: format = GL_RED; break;
		case 3: format = GL_RGB; break;
		case 4: format = GL_RGBA; break;
		}
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

		glBindImageTexture(unit, textureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

		return MakeShared<Texture>(textureID, slotOffset, target);
	}

	Shared<Texture> Texture::LoadTextureHDR(const float* data, uint32_t width, uint32_t height, uint32_t numChannels)
	{
		unsigned int hdrTextureID;
		if (data)
		{
			glGenTextures(1, &hdrTextureID);
			glBindTexture(GL_TEXTURE_2D, hdrTextureID);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, (void*)data);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			Slayer::Log::Error("Failed to load HDR texture!");
		}

		return MakeShared<Texture>(hdrTextureID, 0);
	}

	void Texture::BindTexture(unsigned int textureID, int slot, TextureTarget target)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(target, textureID);
	}

}