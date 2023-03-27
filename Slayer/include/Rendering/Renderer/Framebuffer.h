#pragma once

#include "Core/Core.h"
#include "Rendering/Renderer/Texture.h"

namespace Slayer {

	enum AttachmentTarget : int
	{
		TARGET_NONE,
		RGB16F,
		RG16F,
		RGBA8,
		DEPTHCOMPONENT,
		DEPTHCOMPONENT24,
		DEPTH24STENCIL8
	};

	enum TextureWrap : int
	{
		WRAP_NONE,
		CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
		CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER
	};

	struct Attachment
	{
		unsigned int attachmentID = 0;
		AttachmentTarget attachmentTarget = AttachmentTarget::TARGET_NONE;
		TextureTarget textureTarget = TextureTarget::TYPE_NONE;
		TextureWrap textureWrap = TextureWrap::WRAP_NONE;
		bool generateMips = false;
		Attachment() = default;
		Attachment(AttachmentTarget attachmentTarget)
			: attachmentTarget(attachmentTarget)
		{
			textureTarget = TextureTarget::TEXTURE_2D;
			textureWrap = TextureWrap::CLAMP_TO_EDGE;
		}
		
		Attachment(AttachmentTarget attachmentTarget, TextureTarget textureTarget, TextureWrap textureWrap)
			: attachmentTarget(attachmentTarget), textureTarget(textureTarget), textureWrap(textureWrap)
		{
		}

		Attachment(unsigned int attachmentID, AttachmentTarget attachmentTarget, TextureTarget textureTarget, TextureWrap textureWrap, bool generateMips = false)
			: attachmentID(attachmentID), attachmentTarget(attachmentTarget), textureTarget(textureTarget), textureWrap(textureWrap), generateMips(generateMips)
		{
		}
	};

	class Framebuffer
	{
	private:
		unsigned int fboID;
		unsigned int width;
		unsigned int height;
		Vector<Attachment> colorAttachments;
		Attachment depthAttachment;
	public:
		Framebuffer(unsigned int fboID, 
			Vector<Attachment> colorAttachments,
			Attachment depthAttachment,
			unsigned int width,
			unsigned int height);
		void GenerateMips(int colorAttachmentIndex);
		const Attachment& GetDepthAttachment() { return depthAttachment; }
		unsigned int GetDepthAttachmentID() { return depthAttachment.attachmentID; }
		const Attachment& GetColorAttachment(int index) { return colorAttachments[index]; }
		unsigned int GetColorAttachmentID(int index) { return colorAttachments[index].attachmentID; }
		unsigned int GetFramebufferID() { return fboID; }
		std::pair<unsigned int, unsigned int> GetSize() { return std::make_pair(width, height); }
		void Bind();
		static void Bind(unsigned int fboID, unsigned int width, unsigned int height);
		static void UnbindAll();
		void Unbind();
		void Rebuild();
		void Resize(unsigned int width, unsigned int height);
		static void AttachColorTexture(const Attachment& attachment, GLenum internalFormat, GLenum format, unsigned int width, unsigned int height, int index);
		static void AttachDepthTexture(const Attachment& attachment, GLenum format, GLenum attachmentType, unsigned int width, unsigned int height);
		static Shared<Framebuffer> Create(Vector<Attachment>& colorAttachments, Attachment& depthAttachment, unsigned int width, unsigned int height);
	};

}
