#include "Rendering/Renderer/Framebuffer.h"
#include "glad/glad.h"

namespace Slayer {

	Framebuffer::Framebuffer(unsigned int fboID,
		Vector<Attachment> colorAttachments,
		Attachment depthAttachment,
		unsigned int width,
		unsigned int height) :
		fboID(fboID),
		colorAttachments(colorAttachments),
		depthAttachment(depthAttachment),
		width(width), height(height)

	{
	}

	void Framebuffer::Bind(unsigned int fboID, unsigned int width, unsigned int height)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fboID);
		glViewport(0, 0, width, height);
	}

	void Framebuffer::UnbindAll()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Framebuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fboID);
		glViewport(0, 0, width, height);
	}

	void Framebuffer::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Framebuffer::Rebuild()
	{
	}

	void Framebuffer::Resize(unsigned int inWidth, unsigned int inHeight)
	{
		glDeleteFramebuffers(1, &fboID);
		for (auto& attachment : colorAttachments)
			glDeleteTextures(1, &attachment.attachmentID);
		glDeleteTextures(1, &depthAttachment.attachmentID);

		width = inWidth;
		height = inHeight;

		glGenFramebuffers(1, &fboID);
		glBindFramebuffer(GL_FRAMEBUFFER, fboID);

		if (colorAttachments.size() > 0)
		{
			for (int i = 0; i < colorAttachments.size(); i++)
			{
				glGenTextures(1, &colorAttachments[i].attachmentID);
				glBindTexture(colorAttachments[i].textureTarget, colorAttachments[i].attachmentID);
				switch (colorAttachments[i].attachmentTarget)
				{
				case AttachmentTarget::RGBA8:
					AttachColorTexture(colorAttachments[i], GL_RGB8, GL_RGB, width, height, i);
					break;
				case AttachmentTarget::RGB16F:
					AttachColorTexture(colorAttachments[i], GL_RGB16F, GL_RGB, width, height, i);
					break;
				case AttachmentTarget::RG16F:
					AttachColorTexture(colorAttachments[i], GL_RG16F, GL_RG, width, height, i);
					break;
				}
			}
		}

		glGenTextures(1, &depthAttachment.attachmentID);
		glBindTexture(depthAttachment.textureTarget, depthAttachment.attachmentID);
		//AttachDepthTexture(depthAttachment, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, width, height);

		switch (depthAttachment.attachmentTarget)
		{
		case AttachmentTarget::DEPTH24STENCIL8:
			AttachDepthTexture(depthAttachment, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, width, height);
			break;
		case AttachmentTarget::DEPTHCOMPONENT24:
			AttachDepthTexture(depthAttachment, GL_DEPTH_COMPONENT24, GL_DEPTH_ATTACHMENT, width, height);
			break;
		case AttachmentTarget::DEPTHCOMPONENT:
			AttachDepthTexture(depthAttachment, GL_DEPTH_COMPONENT, GL_DEPTH_ATTACHMENT, width, height);
			break;
		}

		if (colorAttachments.size() > 1)
		{
			Vector<GLenum> buffers;
			for (int i = 0; i < colorAttachments.size(); i++)
			{
				buffers.push_back(GL_COLOR_ATTACHMENT0 + i);
			}
			glDrawBuffers(buffers.size(), buffers.data());
		}
		else if (colorAttachments.empty())
		{
			glDrawBuffer(GL_NONE);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Framebuffer::GenerateMips(int colorAttachmentIndex)
	{
		assert(colorAttachments.size() < colorAttachmentIndex && colorAttachmentIndex > 0);
		glBindTexture(colorAttachments[colorAttachmentIndex].attachmentTarget, colorAttachments[colorAttachmentIndex].attachmentID);
		glGenerateMipmap(colorAttachments[colorAttachmentIndex].attachmentTarget);
		glBindTexture(colorAttachments[colorAttachmentIndex].attachmentTarget, 0);
	}

	void Framebuffer::AttachColorTexture(const Attachment& attachment, GLenum internalFormat, GLenum format, unsigned int width, unsigned int height, int index)
	{
		switch (attachment.textureTarget)
		{
		case TextureTarget::TEXTURE_2D:
			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, attachment.textureWrap);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, attachment.textureWrap);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, attachment.textureWrap);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, attachment.textureTarget, attachment.attachmentID, 0);
			break;
		case TextureTarget::TEXTURE_CUBE_MAP:
			for (unsigned int i = 0; i < 6; ++i)
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, height, 0, format, GL_FLOAT, nullptr);

			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, attachment.textureWrap);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, attachment.textureWrap);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, attachment.textureWrap);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, attachment.generateMips ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			for (unsigned int i = 0; i < 6; ++i)
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, attachment.attachmentID, 0);
			break;
		case TextureTarget::TYPE_NONE:

			break;
		}
	}

	void Framebuffer::AttachDepthTexture(const Attachment& attachment, GLenum format, GLenum attachmentType, unsigned int width, unsigned int height)
	{
		glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, attachment.textureWrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, attachment.textureWrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, attachment.textureWrap);

		glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, GL_TEXTURE_2D, attachment.attachmentID, 0);
	}

	void Framebuffer::Dispose()
	{
		glDeleteFramebuffers(1, &fboID);
		for (auto& attachment : colorAttachments)
			glDeleteTextures(1, &attachment.attachmentID);
		glDeleteTextures(1, &depthAttachment.attachmentID);
	}

	Shared<Framebuffer> Framebuffer::Create(Vector<Attachment>& colorAttachments, Attachment& depthAttachment, unsigned int width, unsigned int height)
	{
		unsigned int fboID;
		glGenFramebuffers(1, &fboID);
		glBindFramebuffer(GL_FRAMEBUFFER, fboID);

		if (colorAttachments.size() > 0)
		{
			for (int i = 0; i < colorAttachments.size(); i++)
			{
				glGenTextures(1, &colorAttachments[i].attachmentID);
				glBindTexture(colorAttachments[i].textureTarget, colorAttachments[i].attachmentID);
				switch (colorAttachments[i].attachmentTarget)
				{
				case AttachmentTarget::RGBA8:
					AttachColorTexture(colorAttachments[i], GL_RGB8, GL_RGB, width, height, i);
					break;
				case AttachmentTarget::RGB16F:
					AttachColorTexture(colorAttachments[i], GL_RGB16F, GL_RGB, width, height, i);
					break;
				case AttachmentTarget::RG16F:
					AttachColorTexture(colorAttachments[i], GL_RG16F, GL_RG, width, height, i);
					break;
				}
			}
		}

		glGenTextures(1, &depthAttachment.attachmentID);
		glBindTexture(depthAttachment.textureTarget, depthAttachment.attachmentID);
		AttachDepthTexture(depthAttachment, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, width, height);

		switch (depthAttachment.attachmentTarget)
		{
		case AttachmentTarget::DEPTH24STENCIL8:
			AttachDepthTexture(depthAttachment, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, width, height);
			break;
		case AttachmentTarget::DEPTHCOMPONENT24:
			AttachDepthTexture(depthAttachment, GL_DEPTH_COMPONENT24, GL_DEPTH_ATTACHMENT, width, height);
			break;
		case AttachmentTarget::DEPTHCOMPONENT:
			AttachDepthTexture(depthAttachment, GL_DEPTH_COMPONENT, GL_DEPTH_ATTACHMENT, width, height);
			break;
		}

		if (colorAttachments.size() > 1)
		{
			Vector<GLenum> buffers;
			for (int i = 0; i < colorAttachments.size(); i++)
			{
				buffers.push_back(GL_COLOR_ATTACHMENT0 + i);
			}
			glDrawBuffers(buffers.size(), buffers.data());
		}
		else if (colorAttachments.empty())
		{
			glDrawBuffer(GL_NONE);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return std::make_shared<Framebuffer>(fboID, colorAttachments, depthAttachment, width, height);
	}

}