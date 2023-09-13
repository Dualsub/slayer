#include "Rendering/Renderer/Renderer.h"

#include "Core/Core.h"
#include "Core/Log.h"

#include "Resources/ResourceManager.h"

#include "Rendering/Renderer/Shader.h"
#include "Rendering/Renderer/Buffer.h"
#include "Rendering/Renderer/Texture.h"
#include "Rendering/Renderer/Camera.h"
#include "Rendering/Renderer/EnvironmentMap.h"

#include "Rendering/Animation/AnimationState.h"

#include "glad/glad.h"
#include "glm/gtc/matrix_transform.hpp"


namespace Slayer
{

	void Renderer::Resize(int width, int height)
	{
		m_camera->SetProjectionMatrix(m_camera->GetFov(), (float)width, (float)height, 20.0f, 10000.0f);
		m_viewportFramebuffer->Resize(width, height);
		glViewport(0, 0, width, height);
	}

	void Renderer::Resize(int x, int y, int width, int height)
	{
		m_camera->SetProjectionMatrix(m_camera->GetFov(), (float)width, (float)height, 20.0f, 10000.0f);
		m_viewportFramebuffer->Resize(width * 2, height * 2);
		glViewport(x, y, width, height);
	}

	void Renderer::Initialize(Shared<Camera> inCamera, int width, int height)
	{
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		// glEnable(GL_CULL_FACE);
		// glCullFace(GL_BACK);

		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		int result;
		glGetIntegerv(GL_MAX_UNIFORM_LOCATIONS, (GLint*)&result);
		Log::Info("Max uniform locations:", result);

		// Framebuffers
		Vector<Attachment> colorAttachments = { { AttachmentTarget::RGBA8 } };
		Attachment depthAttachment = { AttachmentTarget::DEPTH24STENCIL8 };
		m_viewportFramebuffer = Framebuffer::Create(colorAttachments, depthAttachment, width, height);


		// Camera
		m_camera = inCamera;
		m_camera->SetProjectionMatrix(45.0f, (float)width, (float)height, 20.0f, 10000.0f);
		m_cameraBuffer = UniformBuffer::Create(sizeof(CameraData), 0);

		// Instance
		m_instanceBuffer = UniformBuffer::Create(SL_MAX_INSTANCES * sizeof(Mat4), 3);

		// Animation
		m_animationBuffer = UniformBuffer::Create((SL_MAX_INSTANCES * sizeof(AnimationData)) + (SL_MAX_SKELETONS * SL_MAX_BONES * sizeof(int32_t) * 4), 4);
		m_boneBuffer = UniformBuffer::Create(SL_MAX_BONES * sizeof(Mat4), 2);
		const size_t vectorsPerBone = 4;
		const size_t floatsPerVector = 4;
		m_boneTransformTexture = Texture::CreateBuffer(SL_MAX_INSTANCES, SL_MAX_BONES * vectorsPerBone, floatsPerVector, 0, 12);

		// Lights
		m_lightsBuffer = UniformBuffer::Create(sizeof(LightsData), 1);
		m_directionalLight = { Vec3(0.0f), Vec3(0.0f) };
		m_pointLights = {
			{{4.0f, 4.0f, -33.0f}, {1.0f, 0.0f, 0.0f}},
			{{-4.0f, 4.0f, -33.0f}, {0.0f, 1.0f, 0.0f}},
			{{4.0f, -4.0f, -33.0f}, {0.0f, 0.0f, 1.0f}},
			{{-4.0f, -4.0f, -33.0f}, {1.0f, 1.0f, 1.0f}} };

		ResourceManager* rm = ResourceManager::Get();
		m_shaderStatic = rm->GetAsset<Shader>("PBR_shadows_static");
		m_shaderSkeletal = rm->GetAsset<Shader>("PBR_shadows_skeletal");

		m_screenShader = rm->GetAsset<Shader>("ScreenShader");

		m_animationShader = rm->GetAsset<ComputeShader>("SkeletalCompute");

		// Lines
		m_lineVertexArray = VertexArray::Create();
		m_lineVertexBuffer = VertexBuffer::Create(MAX_LINES * 2 * 7 * sizeof(float)); // 3 for Vec3 p and 4 for Vec4 color.
		m_lineVertexBuffer->SetLayout({ {"position", 3}, {"color", 4} });
		m_lineVertexArray->AddVertexBuffer(m_lineVertexBuffer);
		m_lineShader = rm->GetAsset<Shader>("LineShader");
		// PBR
		auto brdfTexture = rm->GetAsset<Texture>("brdf");
		auto prefilterShader = rm->GetAsset<Shader>("PrefilterCapture");
		auto irradianceShader = rm->GetAsset<Shader>("IrradianceConvolution");
		auto captureShader = rm->GetAsset<Shader>("IrradianceCapture");
		auto cubemapShader = rm->GetAsset<Shader>("SimpleCubemap");
		m_hdrTexture = rm->GetAsset<Texture>("DefaultSkybox");
		m_environmentMap = EnvironmentMap::Load(
			m_hdrTexture,
			cubemapShader,
			captureShader,
			irradianceShader,
			prefilterShader,
			brdfTexture
		);

		colorAttachments = Vector<Attachment>();
		depthAttachment = { AttachmentTarget::DEPTHCOMPONENT, TextureTarget::TEXTURE_2D, TextureWrap::CLAMP_TO_BORDER };
		int shadowMapScale = 8;
		m_shadowFramebuffer = Framebuffer::Create(colorAttachments, depthAttachment, shadowMapScale * 512, shadowMapScale * 512);
		m_shadowShaderStatic = rm->GetAsset<Shader>("ShadowMap_static");
		m_shadowShaderSkeletal = rm->GetAsset<Shader>("ShadowMap_skeletal");
		m_lightProjection = glm::ortho(shadowMapScale * -20.0f, shadowMapScale * 20.0f, shadowMapScale * -20.0f, shadowMapScale * 20.0f, 5.0f, shadowMapScale * 200.0f);

		Resize(-width / 2, -height / 2, width / 2, height / 2);
	}

	void Renderer::BeginScene()
	{
		BeginScene(m_lightInfo, m_shadowInfo);
	}

	void Renderer::Clear()
	{
		m_shadowPass.Clear();
		m_mainPass.Clear();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Renderer::BeginScene(const LightInfo& inLightInfo, const ShadowInfo& inShadowInfo)
	{
		m_lightInfo = inLightInfo;
		m_shadowInfo = inShadowInfo;
		CameraData cameraData(m_camera->GetProjectionMatrix(), m_camera->GetViewMatrix(), m_camera->GetPosition());
		m_cameraBuffer->SetData((void*)&cameraData, sizeof(CameraData));

		// shadowFramebuffer->Resize((unsigned int)shadowInfo.width, (unsigned int)shadowInfo.height);
		m_lightProjection = glm::ortho(-m_shadowInfo.width / 2, m_shadowInfo.width / 2, -m_shadowInfo.height / 2, m_shadowInfo.height / 2, m_shadowInfo.near, m_shadowInfo.far);
		m_lightPos = m_shadowInfo.distance * glm::normalize(Vec3(m_lightInfo.directionalLight.direction)) + m_shadowInfo.lightPos;
		Mat4 lightView = glm::lookAt(m_lightPos, m_lightPos + Vec3(m_lightInfo.directionalLight.direction), Vec3(0.0f, 1.0f, 0.0f));
		m_lightSpaceMatrix = m_lightProjection * lightView;
		Vector<PointLight> pointLights;
		LightsData lightsData(m_lightInfo.directionalLight, pointLights, m_lightSpaceMatrix);
		m_lightsBuffer->SetData((void*)&lightsData, sizeof(LightsData));

		m_debugInfo.drawCalls = 0;
	}

	void Renderer::BeginScene(const Vector<PointLight>& inPointLights, const DirectionalLight& inDirectionalLight)
	{
		SL_EVENT();
		CameraData cameraData(m_camera->GetProjectionMatrix(), m_camera->GetViewMatrix(), m_camera->GetPosition());
		m_cameraBuffer->SetData((void*)&cameraData, sizeof(CameraData));
		m_lightPos = -30.0f * glm::normalize(inDirectionalLight.direction);
		Mat4 lightView = glm::lookAt(m_lightPos, m_lightPos + Vec3(inDirectionalLight.direction), Vec3(0.0f, 1.0f, 0.0f));
		m_lightSpaceMatrix = m_lightProjection * lightView;
		LightsData lightsData(inDirectionalLight, inPointLights, m_lightSpaceMatrix);
		m_lightsBuffer->SetData((void*)&lightsData, sizeof(LightsData));

		m_debugInfo.drawCalls = 0;
	}

	void Renderer::SubmitQuad(Shared<Material> material, const Mat4& transform)
	{
		RenderJob job = { Mesh::GetQuadVaoID(),
							Mesh::GetQuadIndexCount(),
							material,
							m_shaderStatic,
							transform };

		m_mainPass.Submit(job);
		m_shadowPass.Submit(job);
	}

	void Renderer::Submit(Shared<SkeletalModel> model, const Mat4& transform, AnimationState* animationState)
	{
		for (auto& mesh : model->GetMeshes())
		{
			RenderJob job = { mesh->GetVaoID(),
								mesh->GetIndexCount(),
								mesh->GetMaterial(),
								m_shaderSkeletal,
								transform,
								animationState };

			m_mainPass.Submit(job);
			m_shadowPass.Submit(job);
		}
	}

	void Renderer::Submit(Shared<SkeletalModel> model, AnimationState* animationState, Shared<Material> material, const Mat4& transform)
	{
		for (auto& mesh : model->GetMeshes())
		{
			RenderJob job = { mesh->GetVaoID(),
								mesh->GetIndexCount(),
								material,
								m_shaderSkeletal,
								transform,
								animationState };

			m_mainPass.Submit(job);
			m_shadowPass.Submit(job);
		}
	}

	void Renderer::Submit(Shared<Model> model, Shared<Material> material, const Mat4& transform)
	{
		for (auto& mesh : model->GetMeshes())
		{
			RenderJob job = { mesh->GetVaoID(),
								mesh->GetIndexCount(),
								material,
								m_shaderStatic,
								transform };
			m_mainPass.Submit(job);
			m_shadowPass.Submit(job);
		}
	}

	void Renderer::Submit(Shared<SkeletalModel> model, Shared<Material> material, const Mat4& transform)
	{
		for (auto& mesh : model->GetMeshes())
		{
			RenderJob job = { mesh->GetVaoID(),
								mesh->GetIndexCount(),
								material,
								m_shaderStatic,
								transform };
			m_mainPass.Submit(job);
			m_shadowPass.Submit(job);
		}
	}

	void Renderer::Submit(Shared<Mesh> mesh, const Mat4& transform)
	{
		RenderJob job = { mesh->GetVaoID(),
							mesh->GetIndexCount(),
							mesh->GetMaterial(),
							m_shaderStatic,
							transform };
		m_mainPass.Submit(job);
		m_shadowPass.Submit(job);
	}

	void Renderer::BindMaterial(Shared<Material> material, Shared<Shader> shader)
	{
		SL_ASSERT(shader && "Shader was null.");

		Map<Slayer::TextureType, AssetID>& textures = material->GetTextures();
		ResourceManager* rm = ResourceManager::Get();

		// Uploading textures.
		std::string typeStr = "";
		for (auto& [type, assetId] : textures)
		{
			Shared<Texture> useTexture = rm->GetAsset<Texture>(assetId);

			switch (type)
			{
			case TextureType::ALBEDO:
				typeStr = "albedo";
				break;
			case TextureType::NORMAL:
				typeStr = "normal";
				break;
			case TextureType::METALLIC:
				typeStr = "metallic";
				break;
			case TextureType::ROUGHNESS:
				typeStr = "roughness";
				break;
			case TextureType::AMBIENT:
				typeStr = "ao";
				break;
			default:
				assert(0 && "Unknown texture type.");
				break;
			}

			shader->SetUniform("material." + typeStr, type);
			useTexture->Bind(type);
		}
	}

	void Renderer::BindAnimation(AnimationState* animationState, Shared<Shader> shader)
	{
		const int animTexSlot = 16;

		SL_ASSERT(shader && "Shader was null.");
		SL_WARN_ASSERT(shader->HasUniform("frames"), "Shader does not have uniform 'frames'.");
		SL_WARN_ASSERT(shader->HasUniform("time"), "Shader does not have uniform 'time'.");
		SL_ASSERT(animationState && "Animation state was null.");
		SL_ASSERT(animationState->inverseBindPose && "Animation state frames were null.");
		SL_ASSERT(animationState->parents && "Animation state parents were null.");

		// Bind uniforms and buffers.
		m_boneBuffer->SetData(animationState->inverseBindPose, SL_MAX_BONES * sizeof(Mat4));
		shader->SetUniform("parents", animationState->parents, SL_MAX_BONES);
		shader->SetUniform("frames", animationState->frames);
		shader->SetUniform("time", animationState->time);

		// Bind animation texture.
		Texture::BindTexture(animationState->textureID, animTexSlot);
		shader->SetUniform("animTex", animTexSlot);
	}

	void Renderer::SubmitLine(Vec3 p1, Vec3 p2, Vec4 color)
	{
		m_lineBuffer.insert(m_lineBuffer.end(), { p1.x, p1.y, p1.z });
		m_lineBuffer.insert(m_lineBuffer.end(), { color.r, color.g, color.b, color.a });
		m_lineBuffer.insert(m_lineBuffer.end(), { p2.x, p2.y, p2.z });
		m_lineBuffer.insert(m_lineBuffer.end(), { color.r, color.g, color.b, color.a });
	}

	void Renderer::Skin()
	{
		SL_EVENT("Skinning Pass");
		AnimationData animationData[SL_MAX_INSTANCES];
		Dict<int32_t*, int32_t> skeletonIds = {};

		{
			SL_EVENT("Animation Data Setup");

			std::memset(animationData, 0, sizeof(AnimationData) * SL_MAX_INSTANCES);

			size_t i = 0;
			for (auto& state : m_mainPass.animationStates)
			{
				if (skeletonIds.find(state.parents) == skeletonIds.end())
				{
					skeletonIds[state.parents] = int32_t(skeletonIds.size());
				}
				int32_t skeletonId = skeletonIds[state.parents];
				animationData[i] = { state.frames, state.time, skeletonId };
				i++;
			}
		}

		const size_t numParents = SL_MAX_BONES * SL_MAX_SKELETONS * 4;
		int32_t parents[numParents];
		const size_t parentsEntrySize = SL_MAX_BONES * sizeof(int32_t);

		{
			SL_EVENT("Parents Data Setup");
			std::memset(parents, 0, numParents * sizeof(int32_t));
			for (auto& [parentPtr, skeletonId] : skeletonIds)
			{
				for (size_t i = 0; i < SL_MAX_BONES; i++)
				{
					parents[skeletonId * SL_MAX_BONES + i * 4] = parentPtr[i];
				}
			}
		}

		{
			SL_EVENT("Animation Buffer Setup");

			m_animationShader->Bind();

			m_animationBuffer->Bind();
			m_animationBuffer->SetSubData(animationData, sizeof(animationData));
			m_animationBuffer->SetSubData(parents, sizeof(parents), sizeof(animationData));
		}

		{
			SL_EVENT("Animation Texture Setup");
			Texture::BindTexture(m_mainPass.animationStates[0].textureID, 0);
			m_animationShader->SetUniform("animTex", 0);
		}

		{
			SL_EVENT("Bone Transform Texture Setup");
			m_boneTransformTexture->Bind();
		}

		{
			SL_EVENT("Compute Shader Dispatch");
			m_animationShader->Dispatch(SL_MAX_BONES, SL_MAX_INSTANCES, 1);
			m_animationShader->MemoryBarrier(MemoryBarrierBits::SL_IMAGE_ACCESS);
		}

		m_boneTransformTexture->Unbind();
		m_animationShader->Unbind();
		m_animationBuffer->Unbind();
	}

	void Renderer::DrawLines()
	{
		if (!m_lineBuffer.size())
			return;

		assert(m_lineBuffer.size() % 7 == 0 && "Number of floats in the buffer is wrong."); // Very good error message.
		m_lineVertexBuffer->SetSubData(m_lineBuffer.data(), m_lineBuffer.size() * sizeof(float));

		m_lineShader->Bind();
		m_lineVertexArray->Bind();

		glClear(GL_DEPTH_BUFFER_BIT);
		glDrawArrays(GL_LINES, 0, m_lineBuffer.size() / 7);
		m_debugInfo.drawCalls++;

		m_lineVertexArray->Unbind();
		m_lineShader->Unbind();

		m_lineBuffer.clear();
	}

	void Renderer::DrawShadows()
	{
		SL_ASSERT(true && "Not implemented.");
		SL_EVENT("Shadow Pass");

		Shared<Shader> shadowShader = nullptr;
		unsigned int currentVao = 0;

		// Shadow pass
		glCullFace(GL_BACK);
		m_shadowFramebuffer->Bind();

		glClear(GL_DEPTH_BUFFER_BIT);

		const auto& jobs = m_mainPass.GetQueue();
		for (auto& job : jobs)
		{
			int32_t wantedShaderID = job.animationState ? m_shadowShaderSkeletal->GetID() : m_shadowShaderStatic->GetID();
			if (shadowShader == nullptr || shadowShader->GetID() != wantedShaderID)
			{
				if (shadowShader)
					shadowShader->Unbind();
				shadowShader = job.animationState ? m_shadowShaderSkeletal : m_shadowShaderStatic;
				shadowShader->BindWithCheck();
				shadowShader->SetUniform("lightSpaceMatrix", m_lightSpaceMatrix);
			}

			if (currentVao != job.vaoID)
			{
				VertexArray::Unbind();
				VertexArray::Bind(job.vaoID);
				currentVao = job.vaoID;
			}

			if (job.animationState)
				BindAnimation(job.animationState, shadowShader);

			shadowShader->SetUniform("transformMatrices", &job.transform, 1);
			glDrawElementsInstanced(GL_TRIANGLES, job.indexCount, GL_UNSIGNED_INT, 0, 1);
			m_debugInfo.drawCalls++;
		}

		shadowShader->Unbind();
		m_shadowFramebuffer->Unbind();
	}

	void Renderer::Draw()
	{
		SL_EVENT("Main Pass");

		m_viewportFramebuffer->Bind();

		// Shared<Shader> currentShader = nullptr;
		// unsigned int currentVao = 0;

		glCullFace(GL_BACK);

		// Main pass

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// const auto& jobs = mainPass.GetQueue();
		// for (auto& job : jobs)
		// {
		// 	if (currentShader == nullptr || currentShader->GetID() != job.shader->GetID())
		// 	{
		// 		if (currentShader)
		// 			currentShader->Unbind();
		// 		currentShader = job.shader;
		// 		currentShader->BindWithCheck();
		// 		currentShader->SetUniform("ibl.irradiance", 0);
		// 		currentShader->SetUniform("ibl.prefilter", 1);
		// 		currentShader->SetUniform("ibl.brdf", 2);
		// 		currentShader->SetUniform("shadowMap", 3);
		// 		Texture::BindTexture(environmentMap->GetIrradianceMapID(), 0, TextureTarget::TEXTURE_CUBE_MAP);
		// 		Texture::BindTexture(environmentMap->GetPrefilterMapID(), 1, TextureTarget::TEXTURE_CUBE_MAP);
		// 		Texture::BindTexture(environmentMap->GetBRDFTextureID(), 2, TextureTarget::TEXTURE_2D);
		// 		Texture::BindTexture(shadowFramebuffer->GetDepthAttachmentID(), 3, TextureTarget::TEXTURE_2D);
		// 	}
		// 	if (currentVao != job.vaoID)
		// 	{
		// 		VertexArray::Unbind();
		// 		VertexArray::Bind(job.vaoID);
		// 		currentVao = job.vaoID;
		// 	}

		// 	BindMaterial(job.material, currentShader);

		// 	if (job.animationState)
		// 		BindAnimation(job.animationState, currentShader);

		// 	currentShader->SetUniform("transformMatrices", &job.transform, 1);
		// 	glDrawElementsInstanced(GL_TRIANGLES, job.indexCount, GL_UNSIGNED_INT, 0, 1);
		// 	debugInfo.drawCalls++;
		// }

		const auto& batches = m_mainPass.GetBatches();
		for (auto& batch : batches)
		{
			Shared<Shader> currentShader = batch.shader;
			{
				SL_GPU_EVENT("Shader Setup");
				currentShader->Bind();
				currentShader->SetUniform("ibl.irradiance", 0);
				currentShader->SetUniform("ibl.prefilter", 1);
				currentShader->SetUniform("ibl.brdf", 2);
				currentShader->SetUniform("shadowMap", 3);
				Texture::BindTexture(m_environmentMap->GetIrradianceMapID(), 0, TextureTarget::TEXTURE_CUBE_MAP);
				Texture::BindTexture(m_environmentMap->GetPrefilterMapID(), 1, TextureTarget::TEXTURE_CUBE_MAP);
				Texture::BindTexture(m_environmentMap->GetBRDFTextureID(), 2, TextureTarget::TEXTURE_2D);
				Texture::BindTexture(m_shadowFramebuffer->GetDepthAttachmentID(), 3, TextureTarget::TEXTURE_2D);

				currentShader->SetUniform("boneTransformTex", 12);
				m_boneTransformTexture->Bind();
			}

			{
				SL_GPU_EVENT("VAO Bind");
				VertexArray::Unbind();
				VertexArray::Bind(batch.vaoID);
			}

			{
				SL_GPU_EVENT("Material Bind");
				BindMaterial(batch.material, currentShader);
			}

			{
				SL_GPU_EVENT("Instance buffer");
				const size_t instanceDataSize = SL_MAX_INSTANCES * sizeof(Mat4);
				m_instanceBuffer->SetData(batch.transforms.data(), batch.transforms.size() * sizeof(Mat4));
				m_boneBuffer->SetData(batch.inverseBindPose, SL_MAX_BONES * sizeof(Mat4));
			}

			{
				SL_GPU_EVENT("Draw call");
				glDrawElementsInstanced(GL_TRIANGLES, batch.indexCount, GL_UNSIGNED_INT, 0, (GLsizei)batch.transforms.size());
			}

			m_debugInfo.drawCalls++;
		}

		m_environmentMap->Draw();

		m_viewportFramebuffer->Unbind();
	}

	void Renderer::EndScene()
	{
		SL_EVENT();
		glClear(GL_COLOR_BUFFER_BIT);

		m_screenShader->Bind();
		m_screenShader->SetUniform("screenTexture", 0);
		glBindVertexArray(Mesh::GetQuadVaoID());
		// glDisable(GL_DEPTH_TEST);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_viewportFramebuffer->GetColorAttachmentID(0));
		glDrawArrays(GL_TRIANGLES, 0, 6);
		m_screenShader->Unbind();
	}

	void Renderer::CleanUp()
	{
	}

	void Renderer::SetActiveCamera(Shared<Camera> inCamera, const Vec2& windowSize)
	{
		m_camera = inCamera;
		Resize(-windowSize.x / 2, -windowSize.y / 2, windowSize.x / 2, windowSize.y / 2);
	}

	RenderJob::RenderJob(unsigned int vaoID, unsigned int indexCount, Shared<Material> material, Shared<Shader> shader, const Mat4& transform) : vaoID(vaoID), indexCount(indexCount), material(material), shader(shader), transform(transform)
	{
	}

	RenderJob::RenderJob(unsigned int vaoID, unsigned int indexCount, Shared<Material> material, Shared<Shader> shader, const Mat4& transform, AnimationState* animationState) : vaoID(vaoID), indexCount(indexCount), material(material), shader(shader), transform(transform), animationState(animationState)
	{
	}

}
