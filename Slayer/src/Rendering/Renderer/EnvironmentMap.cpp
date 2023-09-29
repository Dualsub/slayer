#include "Rendering/Renderer/EnvironmentMap.h"
#include "Rendering/Renderer/Mesh.h"
#include "Rendering/Renderer/Shader.h"
#include "Rendering/Renderer/Framebuffer.h"

#include "glad/glad.h"
#include "glm/gtc/matrix_transform.hpp"

namespace Slayer {

	Shared<VertexArray> EnvironmentMap::cubeVAO;
	Shared<VertexArray> EnvironmentMap::quadVAO;

	unsigned int EnvironmentMap::CreateEnvironmentMap(unsigned int fboID, unsigned int rboID, Shared<Texture> hdrTexture, Shared<Shader> captureShader, const Mat4& captureProjection, const Mat4 captureViews[6])
	{
		unsigned int envCubemapID;
		unsigned int width = 512 * 2;
		unsigned int height = 512 * 2;

		glBindFramebuffer(GL_FRAMEBUFFER, fboID);
		glBindRenderbuffer(GL_RENDERBUFFER, rboID);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboID);

		glGenTextures(1, &envCubemapID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemapID);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
				width, height, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		captureShader->Bind();
		captureShader->SetUniform("equirectangularMap", 0);
		captureShader->SetUniform("projection", captureProjection);

		hdrTexture->Bind();

		glViewport(0, 0, width, height);
		glBindFramebuffer(GL_FRAMEBUFFER, fboID);

		for (unsigned int i = 0; i < 6; ++i)
		{
			captureShader->SetUniform("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemapID, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			cubeVAO->Bind();
			glDrawArrays(GL_TRIANGLES, 0, 36);
			cubeVAO->Unbind();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemapID);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		return envCubemapID;
	}

	unsigned int EnvironmentMap::CreateIrradianceMap(unsigned int fboID, unsigned int rboID, unsigned int envCubemapID, Shared<Shader> irradianceShader, const Mat4& captureProjection, const Mat4 captureViews[6])
	{
		unsigned int irradianceMapID;
		glGenTextures(1, &irradianceMapID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMapID);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindFramebuffer(GL_FRAMEBUFFER, fboID);
		glBindRenderbuffer(GL_RENDERBUFFER, rboID);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

		irradianceShader->Bind();
		irradianceShader->SetUniform("environmentMap", 0);
		irradianceShader->SetUniform("projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemapID);

		glViewport(0, 0, 32, 32);
		glBindFramebuffer(GL_FRAMEBUFFER, fboID);
		for (unsigned int i = 0; i < 6; ++i)
		{
			irradianceShader->SetUniform("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMapID, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			cubeVAO->Bind();
			glDrawArrays(GL_TRIANGLES, 0, 36);
			cubeVAO->Unbind();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		irradianceShader->Unbind();
		return irradianceMapID;
	}

	unsigned int EnvironmentMap::CreatePrefilterMap(unsigned int fboID, unsigned int rboID, unsigned int envCubemapID, Shared<Shader> prefilterShader, const Mat4& captureProjection, const Mat4 captureViews[6])
	{
		unsigned int prefilterMapID;
		glGenTextures(1, &prefilterMapID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMapID);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		prefilterShader->Bind();
		prefilterShader->SetUniform("environmentMap", 0);
		prefilterShader->SetUniform("projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemapID);

		glBindFramebuffer(GL_FRAMEBUFFER, fboID);
		unsigned int maxMipLevels = 5;
		for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
		{
			// reisze framebuffer according to mip-level size.
			unsigned int mipWidth = (unsigned int)(128 * std::pow(0.5, mip));
			unsigned int mipHeight = (unsigned int)(128 * std::pow(0.5, mip));
			glBindRenderbuffer(GL_RENDERBUFFER, rboID);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
			glViewport(0, 0, mipWidth, mipHeight);
			glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMapID);

			float roughness = (float)mip / (float)(maxMipLevels - 1);
			prefilterShader->SetUniform("roughness", roughness);
			for (unsigned int i = 0; i < 6; ++i)
			{
				prefilterShader->SetUniform("view", captureViews[i]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMapID, mip);

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				cubeVAO->Bind();
				glDrawArrays(GL_TRIANGLES, 0, 36);
				cubeVAO->Unbind();
			}
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		prefilterShader->Unbind();

		return prefilterMapID;
	}

	unsigned int EnvironmentMap::CreateBRDF(unsigned int fboID, unsigned int rboID, Shared<Shader> brdfShader)
	{
		Vector<Attachment> colorAttachments = { { AttachmentTarget::RGB16F } };
		Attachment depth = { AttachmentTarget::DEPTHCOMPONENT24 };
		Shared<Framebuffer> fb = Framebuffer::Create(colorAttachments, depth, 512, 512);

		fb->Bind();
		brdfShader->Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		quadVAO->Bind();
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		quadVAO->Unbind();
		brdfShader->Unbind();
		fb->Unbind();

		return colorAttachments[0].attachmentID;
	}

	Shared<EnvironmentMap> EnvironmentMap::Load(Shared<Texture> hdrTexture, Shared<Shader> cubemapShader, Shared<Shader> captureShader, Shared<Shader> irradianceShader, Shared<Shader> prefilterShader, Shared<Texture> brdfTexture)
	{
		glDisable(GL_CULL_FACE);

		// Load Cube if it hasn't been done.
		if (!cubeVAO)
		{
			float cubeVertices[] = {
				// back face
				-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
				 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
				 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
				 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
				-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
				-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
				// front face
				-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
				 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
				 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
				 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
				-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
				-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
				// left face
				-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
				-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
				-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
				-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
				-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
				-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
				// right face
				 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
				 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
				 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
				 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
				 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
				 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
				 // bottom face
				 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
				  1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
				  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
				  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
				 -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
				 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
				 // top face
				 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
				  1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
				  1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
				  1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
				 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
				 -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
			};
			auto cubeVBO = VertexBuffer::Create(cubeVertices, sizeof(cubeVertices));
			cubeVBO->SetLayout({ {"position", 3}, {"normal", 3}, {"texcoord", 2} });
			cubeVAO = VertexArray::Create();
			cubeVAO->AddVertexBuffer(cubeVBO);
			cubeVBO->Unbind();
		}

		if (!quadVAO)
		{
			float quadVertices[] = {
				-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
				 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
				 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			};

			auto quadVBO = VertexBuffer::Create(quadVertices, sizeof(quadVertices));
			quadVBO->SetLayout({ {"position", 3}, {"texcoord", 2} });
			quadVAO = VertexArray::Create();
			quadVAO->AddVertexBuffer(quadVBO);
			quadVBO->Unbind();
		}

		unsigned int fboID;
		unsigned int rboID;
		glGenFramebuffers(1, &fboID);
		glGenRenderbuffers(1, &rboID);

		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		glm::mat4 captureViews[] =
		{
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};

		unsigned int envCubemapID;
		envCubemapID = EnvironmentMap::CreateEnvironmentMap(fboID, rboID, hdrTexture, captureShader, captureProjection, captureViews);

		unsigned int irradianceMapID;
		irradianceMapID = EnvironmentMap::CreateIrradianceMap(fboID, rboID, envCubemapID, irradianceShader, captureProjection, captureViews);

		unsigned int prefilterMapID;
		prefilterMapID = EnvironmentMap::CreatePrefilterMap(fboID, rboID, envCubemapID, prefilterShader, captureProjection, captureViews);

		// Reset viewport.
		glEnable(GL_CULL_FACE);

		return MakeShared<EnvironmentMap>(envCubemapID, irradianceMapID, prefilterMapID, brdfTexture->GetTextureID(), cubemapShader);
	}

	EnvironmentMap::EnvironmentMap(unsigned int envCubemapID, unsigned int irradianceMapID, unsigned int prefilterMapID, unsigned int brdfTextureID, Shared<Shader> cubemapShader)
		: envCubemapID(envCubemapID), irradianceMapID(irradianceMapID), prefilterMapID(prefilterMapID), brdfTextureID(brdfTextureID), cubemapShader(cubemapShader)
	{
	}

	void EnvironmentMap::Draw()
	{
		glCullFace(GL_FRONT);
		cubemapShader->Bind();
		cubemapShader->SetUniform("environmentMap", 16);

		Texture::BindTexture(envCubemapID, 16, TextureTarget::TEXTURE_CUBE_MAP);

		cubeVAO->Bind();
		glDrawArrays(GL_TRIANGLES, 0, 36);
		cubeVAO->Unbind();

		cubemapShader->Unbind();
		glCullFace(GL_BACK);
	}

	void EnvironmentMap::Dispose()
	{
		glDeleteTextures(1, &envCubemapID);
		glDeleteTextures(1, &irradianceMapID);
		glDeleteTextures(1, &prefilterMapID);
		glDeleteTextures(1, &brdfTextureID);
	}
}