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
		m_viewportFramebuffer->Resize(width, height);
		glViewport(0, 0, width, height);
	}

	void Renderer::Resize(int x, int y, int width, int height)
	{
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
		Vector<Attachment> colorAttachments = { { AttachmentTarget::RGBA16F } };
		Attachment depthAttachment = { AttachmentTarget::DEPTH24STENCIL8 };
		m_viewportFramebuffer = Framebuffer::Create(colorAttachments, depthAttachment, width, height);


		// Camera
		m_cameraBuffer = UniformBuffer::Create(sizeof(CameraData), 0);

		// Instance
		m_instanceBuffer = UniformBuffer::Create(SL_MAX_INSTANCES * (sizeof(Mat4) + 4 * sizeof(int32_t)), 3);

		// Animation
		m_animationBuffer = UniformBuffer::Create((SL_MAX_INSTANCES * sizeof(AnimationBuffer)) + (SL_MAX_SKELETONS * SL_MAX_BONES * sizeof(int32_t) * 4), 4);
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
		int shadowMapScale = 16;
		m_shadowFramebuffer = Framebuffer::Create(colorAttachments, depthAttachment, shadowMapScale * 512, shadowMapScale * 512);
		m_shadowShaderStatic = rm->GetAsset<Shader>("ShadowMap_static");
		m_shadowShaderSkeletal = rm->GetAsset<Shader>("ShadowMap_skeletal");
		m_lightProjection = glm::ortho(shadowMapScale * -20.0f, shadowMapScale * 20.0f, shadowMapScale * -20.0f, shadowMapScale * 20.0f, 5.0f, shadowMapScale * 200.0f);

		Resize(-width / 2, -height / 2, width / 2, height / 2);
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
		BeginScene();
	}

	void Renderer::BeginScene()
	{
		SL_EVENT();
		m_cameraBuffer->SetData((void*)&m_cameraData, sizeof(CameraData));
		m_lightPos = -600.0f * glm::normalize(m_directionalLight.direction);
		Mat4 lightView = glm::lookAt(m_lightPos, m_lightPos + Vec3(m_directionalLight.direction), Vec3(0.0f, 1.0f, 0.0f));
		m_lightSpaceMatrix = m_lightProjection * lightView;
		LightsData lightsData(m_directionalLight, {}, m_lightSpaceMatrix);
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

	void Renderer::Submit(Shared<Model> model, const Vector<Shared<Material>>& materials, const Mat4& transform)
	{
		auto& meshes = model->GetMeshes();
		for (uint32_t i = 0; i < meshes.size(); i++)
		{
			if (i >= materials.size())
				break;
			if (!materials[i])
				continue;

			auto& mesh = meshes[i];
			RenderJob job = { mesh->GetVaoID(),
								mesh->GetIndexCount(),
								materials[i],
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

		if (m_mainPass.animationStates.size() == 0)
		{
			return;
		}

		AnimationBuffer animationBuffer[SL_MAX_INSTANCES];
		Dict<int32_t*, int32_t> skeletonIds = {};
		FixedVector<int32_t, SL_MAX_ANIMATIONS> animationTextures = {};

		{
			SL_EVENT("Animation Data Setup");

			std::memset(animationBuffer, 0, sizeof(AnimationBuffer) * SL_MAX_INSTANCES);

			size_t i = 0;
			for (auto& state : m_mainPass.animationStates)
			{
				if (skeletonIds.find(state.parents) == skeletonIds.end())
				{
					skeletonIds[state.parents] = int32_t(skeletonIds.size());
				}
				int32_t skeletonId = skeletonIds[state.parents];
				int32_t animationIds[SL_MAX_BLEND_ANIMATIONS] = { -1 };
				for (auto j = 0; j < SL_MAX_BLEND_ANIMATIONS; j++)
				{
					if (state.textureIDs[j] == -1)
						continue;

					animationIds[j] = (int32_t)animationTextures.FindIndexOf(state.textureIDs[j]);
					if (animationIds[j] == -1)
					{
						animationTextures.PushBack(state.textureIDs[j]);
						animationIds[j] = animationTextures.Size() - 1;
					}
				}

				animationBuffer[i] = AnimationBuffer(skeletonId, animationIds, state.weights, state.times, state.frames);
				i++;
			}
		}

		const size_t numParents = SL_MAX_BONES * SL_MAX_SKELETONS * 4;
		int32_t parents[numParents];

		{
			SL_EVENT("Parents Data Setup");
			std::memset(parents, 0, numParents * sizeof(int32_t));
			for (auto& [parentPtr, skeletonId] : skeletonIds)
			{
				for (size_t i = 0; i < SL_MAX_BONES; i++)
				{
					parents[skeletonId * SL_MAX_BONES * 4 + i * 4] = parentPtr[i];
				}
			}
		}

		{
			SL_EVENT("Animation Buffer Setup");

			m_animationShader->Bind();

			m_animationBuffer->Bind();
			m_animationBuffer->SetSubData(animationBuffer, sizeof(animationBuffer));
			m_animationBuffer->SetSubData(parents, sizeof(parents), sizeof(animationBuffer));
		}

		{
			SL_EVENT("Animation Texture Setup");
			for (size_t i = 0; i < animationTextures.Size(); i++)
			{
				Texture::BindTexture(animationTextures[i], i);
				m_animationShader->SetUniform("animTextures[" + std::to_string(i) + "]", i);
			}
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

	void Renderer::BindInstanceBuffer(const FixedVector<int32_t, SL_MAX_INSTANCES>& animInstanceIds, const FixedVector<Mat4, SL_MAX_INSTANCES>& transforms)
	{
		const size_t transformsSize = SL_MAX_INSTANCES * sizeof(Mat4);
		m_instanceBuffer->Bind();
		m_instanceBuffer->SetSubData(transforms.Data(), transformsSize);

		int32_t animInstanceIdsArray[SL_MAX_INSTANCES * 4]; // We create the array with 12 bytes of padding per instance. :/
		std::memset(animInstanceIdsArray, -1, sizeof(int32_t) * SL_MAX_INSTANCES * 4);
		for (size_t i = 0; i < animInstanceIds.Size(); i++)
		{
			animInstanceIdsArray[i * 4] = animInstanceIds[i];
		}

		m_instanceBuffer->SetSubData(animInstanceIdsArray, sizeof(int32_t) * SL_MAX_INSTANCES * 4, transformsSize);
		m_instanceBuffer->Unbind();
	}

	void Renderer::DrawShadows()
	{
		SL_ASSERT(true && "Not implemented.");
		SL_EVENT("Shadow Pass");

		// Shadow pass
		glCullFace(GL_FRONT);
		m_shadowFramebuffer->Bind();

		glClear(GL_DEPTH_BUFFER_BIT);

		const auto& batches = m_mainPass.GetBatches();
		for (auto& batch : batches)
		{
			Shared<Shader> currentShader = batch.inverseBindPose ? m_shadowShaderSkeletal : m_shadowShaderStatic;
			{
				SL_GPU_EVENT("Shader Setup");
				currentShader->Bind();

				currentShader->SetUniform("boneTransformTex", 12);
				m_boneTransformTexture->Bind();

				currentShader->SetUniform("lightSpaceMatrix", m_lightSpaceMatrix);
			}

			{
				SL_GPU_EVENT("VAO Bind");
				VertexArray::Unbind();
				VertexArray::Bind(batch.vaoID);
			}

			{
				SL_GPU_EVENT("Instance buffer");
				BindInstanceBuffer(batch.animInstanceIds, batch.transforms);
				if (batch.inverseBindPose)
					m_boneBuffer->SetData(batch.inverseBindPose, SL_MAX_BONES * sizeof(Mat4));
			}

			{
				SL_GPU_EVENT("Draw call");
				glDrawElementsInstanced(GL_TRIANGLES, batch.indexCount, GL_UNSIGNED_INT, 0, (GLsizei)batch.transforms.Size());
			}

			m_debugInfo.drawCalls++;
		}

		m_shadowFramebuffer->Unbind();
	}

	void Renderer::Draw()
	{
		SL_EVENT("Main Pass");

		m_viewportFramebuffer->Bind();

		glCullFace(GL_BACK);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
				BindInstanceBuffer(batch.animInstanceIds, batch.transforms);
				if (batch.inverseBindPose)
					m_boneBuffer->SetData(batch.inverseBindPose, SL_MAX_BONES * sizeof(Mat4));
			}

			{
				SL_GPU_EVENT("Draw call");
				glDrawElementsInstanced(GL_TRIANGLES, batch.indexCount, GL_UNSIGNED_INT, 0, (GLsizei)batch.transforms.Size());
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
		m_screenShader->SetUniform("exposure", m_exposure);
		m_screenShader->SetUniform("gamma", m_gamma);
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
		m_screenShader->Dispose();
		m_viewportFramebuffer->Dispose();
		m_cameraBuffer->Dispose();
		m_boneBuffer->Dispose();
		m_instanceBuffer->Dispose();
		m_lightsBuffer->Dispose();
		m_lineVertexArray->Dispose();
		m_lineVertexBuffer->Dispose();
		m_lineShader->Dispose();
		m_shadowFramebuffer->Dispose();
		m_shadowShaderStatic->Dispose();
		m_shadowShaderSkeletal->Dispose();
		m_animationShader->Dispose();
		m_animationBuffer->Dispose();
		m_boneTransformTexture->Dispose();
		m_hdrTexture->Dispose();
		m_environmentMap->Dispose();
		m_shaderStatic->Dispose();
		m_shaderSkeletal->Dispose();
	}

	void Renderer::SetCameraData(const Mat4& projectionMatrix, const Mat4& viewMatrix, const Vec3& position)
	{
		m_cameraData.Set(projectionMatrix, viewMatrix, position);
	}

	void Renderer::SetDirectionalLight(const Vec3& orientation, const Vec3& color)
	{
		Vec3 direction = Vec3(0.0f, -1.0f, 0.0f);
		direction = glm::mat3_cast(Quat(glm::radians(orientation))) * direction;
		m_directionalLight = { direction, color };
	}

	RenderJob::RenderJob(unsigned int vaoID, unsigned int indexCount, Shared<Material> material, Shared<Shader> shader, const Mat4& transform) : vaoID(vaoID), indexCount(indexCount), material(material), shader(shader), transform(transform)
	{
	}

	RenderJob::RenderJob(unsigned int vaoID, unsigned int indexCount, Shared<Material> material, Shared<Shader> shader, const Mat4& transform, AnimationState* animationState) : vaoID(vaoID), indexCount(indexCount), material(material), shader(shader), transform(transform), animationState(animationState)
	{
	}

}
