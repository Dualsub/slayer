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
		camera->SetProjectionMatrix(camera->GetFov(), (float)width, (float)height, 20.0f, 10000.0f);
		viewportFramebuffer->Resize(width, height);
		glViewport(0, 0, width, height);
	}

	void Renderer::Resize(int x, int y, int width, int height)
	{
		camera->SetProjectionMatrix(camera->GetFov(), (float)width, (float)height, 20.0f, 10000.0f);
		viewportFramebuffer->Resize(width * 2, height * 2);
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
		viewportFramebuffer = Framebuffer::Create(colorAttachments, depthAttachment, width, height);


		// Camera
		camera = inCamera;
		camera->SetProjectionMatrix(45.0f, (float)width, (float)height, 20.0f, 10000.0f);
		cameraBuffer = UniformBuffer::Create(sizeof(CameraData), 0);

		// Animation
		instanceBuffer = UniformBuffer::Create(SL_MAX_INSTANCES * (sizeof(Mat4) + sizeof(Batch::InstanceData)), 3);
		boneBuffer = UniformBuffer::Create(SL_MAX_BONES * (sizeof(Mat4) + 4 * sizeof(int32_t)), 2);

		// Lights
		lightsBuffer = UniformBuffer::Create(sizeof(LightsData), 1);
		directionalLight = { Vec3(0.0f), Vec3(0.0f) };
		pointLights = {
			{{4.0f, 4.0f, -33.0f}, {1.0f, 0.0f, 0.0f}},
			{{-4.0f, 4.0f, -33.0f}, {0.0f, 1.0f, 0.0f}},
			{{4.0f, -4.0f, -33.0f}, {0.0f, 0.0f, 1.0f}},
			{{-4.0f, -4.0f, -33.0f}, {1.0f, 1.0f, 1.0f}} };

		ResourceManager* rm = ResourceManager::Get();
		shaderStatic = rm->GetAsset<Shader>("PBR_shadows_static");
		shaderSkeletal = rm->GetAsset<Shader>("PBR_shadows_skeletal");

		screenShader = rm->GetAsset<Shader>("ScreenShader");

		// Lines
		lineVertexArray = VertexArray::Create();
		lineVertexBuffer = VertexBuffer::Create(MAX_LINES * 2 * 7 * sizeof(float)); // 3 for Vec3 p and 4 for Vec4 color.
		lineVertexBuffer->SetLayout({ {"position", 3}, {"color", 4} });
		lineVertexArray->AddVertexBuffer(lineVertexBuffer);
		lineShader = rm->GetAsset<Shader>("LineShader");
		// PBR
		auto brdfTexture = rm->GetAsset<Texture>("brdf");
		auto prefilterShader = rm->GetAsset<Shader>("PrefilterCapture");
		auto irradianceShader = rm->GetAsset<Shader>("IrradianceConvolution");
		auto captureShader = rm->GetAsset<Shader>("IrradianceCapture");
		auto cubemapShader = rm->GetAsset<Shader>("SimpleCubemap");
		hdrTexture = rm->GetAsset<Texture>("DefaultSkybox");
		environmentMap = EnvironmentMap::Load(
			hdrTexture,
			cubemapShader,
			captureShader,
			irradianceShader,
			prefilterShader,
			brdfTexture
		);

		colorAttachments = Vector<Attachment>();
		depthAttachment = { AttachmentTarget::DEPTHCOMPONENT, TextureTarget::TEXTURE_2D, TextureWrap::CLAMP_TO_BORDER };
		int shadowMapScale = 8;
		shadowFramebuffer = Framebuffer::Create(colorAttachments, depthAttachment, shadowMapScale * 512, shadowMapScale * 512);
		shadowShaderStatic = rm->GetAsset<Shader>("ShadowMap_static");
		shadowShaderSkeletal = rm->GetAsset<Shader>("ShadowMap_skeletal");
		lightProjection = glm::ortho(shadowMapScale * -20.0f, shadowMapScale * 20.0f, shadowMapScale * -20.0f, shadowMapScale * 20.0f, 5.0f, shadowMapScale * 200.0f);

		Resize(-width / 2, -height / 2, width / 2, height / 2);
	}

	void Renderer::BeginScene()
	{
		BeginScene(lightInfo, shadowInfo);
	}

	void Renderer::Clear()
	{
		shadowPass.Clear();
		mainPass.Clear();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Renderer::BeginScene(const LightInfo& inLightInfo, const ShadowInfo& inShadowInfo)
	{
		lightInfo = inLightInfo;
		shadowInfo = inShadowInfo;
		CameraData cameraData(camera->GetProjectionMatrix(), camera->GetViewMatrix(), camera->GetPosition());
		cameraBuffer->SetData((void*)&cameraData, sizeof(CameraData));

		// shadowFramebuffer->Resize((unsigned int)shadowInfo.width, (unsigned int)shadowInfo.height);
		lightProjection = glm::ortho(-shadowInfo.width / 2, shadowInfo.width / 2, -shadowInfo.height / 2, shadowInfo.height / 2, shadowInfo.near, shadowInfo.far);
		lightPos = shadowInfo.distance * glm::normalize(Vec3(lightInfo.directionalLight.direction)) + shadowInfo.lightPos;
		Mat4 lightView = glm::lookAt(lightPos, lightPos + Vec3(lightInfo.directionalLight.direction), Vec3(0.0f, 1.0f, 0.0f));
		lightSpaceMatrix = lightProjection * lightView;
		Vector<PointLight> pointLights;
		LightsData lightsData(lightInfo.directionalLight, pointLights, lightSpaceMatrix);
		lightsBuffer->SetData((void*)&lightsData, sizeof(LightsData));

		debugInfo.drawCalls = 0;
	}

	void Renderer::BeginScene(const Vector<PointLight>& inPointLights, const DirectionalLight& inDirectionalLight)
	{
		SL_EVENT();
		CameraData cameraData(camera->GetProjectionMatrix(), camera->GetViewMatrix(), camera->GetPosition());
		cameraBuffer->SetData((void*)&cameraData, sizeof(CameraData));
		lightPos = -30.0f * glm::normalize(inDirectionalLight.direction);
		Mat4 lightView = glm::lookAt(lightPos, lightPos + Vec3(inDirectionalLight.direction), Vec3(0.0f, 1.0f, 0.0f));
		lightSpaceMatrix = lightProjection * lightView;
		LightsData lightsData(inDirectionalLight, inPointLights, lightSpaceMatrix);
		lightsBuffer->SetData((void*)&lightsData, sizeof(LightsData));

		debugInfo.drawCalls = 0;
	}

	void Renderer::SubmitQuad(Shared<Material> material, const Mat4& transform)
	{
		RenderJob job = { Mesh::GetQuadVaoID(),
							Mesh::GetQuadIndexCount(),
							material,
							shaderStatic,
							transform };

		mainPass.Submit(job);
		shadowPass.Submit(job);
	}

	void Renderer::Submit(Shared<SkeletalModel> model, const Mat4& transform, AnimationState* animationState)
	{
		for (auto& mesh : model->GetMeshes())
		{
			RenderJob job = { mesh->GetVaoID(),
								mesh->GetIndexCount(),
								mesh->GetMaterial(),
								shaderSkeletal,
								transform,
								animationState };

			mainPass.Submit(job);
			shadowPass.Submit(job);
		}
	}

	void Renderer::Submit(Shared<SkeletalModel> model, AnimationState* animationState, Shared<Material> material, const Mat4& transform)
	{
		for (auto& mesh : model->GetMeshes())
		{
			RenderJob job = { mesh->GetVaoID(),
								mesh->GetIndexCount(),
								material,
								shaderSkeletal,
								transform,
								animationState };

			mainPass.Submit(job);
			shadowPass.Submit(job);
		}
	}

	void Renderer::Submit(Shared<Model> model, Shared<Material> material, const Mat4& transform)
	{
		for (auto& mesh : model->GetMeshes())
		{
			RenderJob job = { mesh->GetVaoID(),
								mesh->GetIndexCount(),
								material,
								shaderStatic,
								transform };
			mainPass.Submit(job);
			shadowPass.Submit(job);
		}
	}

	void Renderer::Submit(Shared<SkeletalModel> model, Shared<Material> material, const Mat4& transform)
	{
		for (auto& mesh : model->GetMeshes())
		{
			RenderJob job = { mesh->GetVaoID(),
								mesh->GetIndexCount(),
								material,
								shaderStatic,
								transform };
			mainPass.Submit(job);
			shadowPass.Submit(job);
		}
	}

	void Renderer::Submit(Shared<Mesh> mesh, const Mat4& transform)
	{
		RenderJob job = { mesh->GetVaoID(),
							mesh->GetIndexCount(),
							mesh->GetMaterial(),
							shaderStatic,
							transform };
		mainPass.Submit(job);
		shadowPass.Submit(job);
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
		boneBuffer->SetData(animationState->inverseBindPose, SL_MAX_BONES * sizeof(Mat4));
		shader->SetUniform("parents", animationState->parents, SL_MAX_BONES);
		shader->SetUniform("frames", animationState->frames);
		shader->SetUniform("time", animationState->time);

		// Bind animation texture.
		Texture::BindTexture(animationState->textureID, animTexSlot);
		shader->SetUniform("animTex", animTexSlot);
	}

	void Renderer::SubmitLine(Vec3 p1, Vec3 p2, Vec4 color)
	{
		lineBuffer.insert(lineBuffer.end(), { p1.x, p1.y, p1.z });
		lineBuffer.insert(lineBuffer.end(), { color.r, color.g, color.b, color.a });
		lineBuffer.insert(lineBuffer.end(), { p2.x, p2.y, p2.z });
		lineBuffer.insert(lineBuffer.end(), { color.r, color.g, color.b, color.a });
	}

	void Renderer::DrawLines()
	{
		if (!lineBuffer.size())
			return;

		assert(lineBuffer.size() % 7 == 0 && "Number of floats in the buffer is wrong."); // Very good error message.
		lineVertexBuffer->SetSubData(lineBuffer.data(), lineBuffer.size() * sizeof(float));

		lineShader->Bind();
		lineVertexArray->Bind();

		glClear(GL_DEPTH_BUFFER_BIT);
		glDrawArrays(GL_LINES, 0, lineBuffer.size() / 7);
		debugInfo.drawCalls++;

		lineVertexArray->Unbind();
		lineShader->Unbind();

		lineBuffer.clear();
	}

	void Renderer::DrawShadows()
	{
		SL_EVENT("Shadow Pass");

		Shared<Shader> shadowShader = nullptr;
		unsigned int currentVao = 0;

		// Shadow pass
		glCullFace(GL_BACK);
		shadowFramebuffer->Bind();

		glClear(GL_DEPTH_BUFFER_BIT);

		const auto& jobs = mainPass.GetQueue();
		for (auto& job : jobs)
		{
			int32_t wantedShaderID = job.animationState ? shadowShaderSkeletal->GetID() : shadowShaderStatic->GetID();
			if (shadowShader == nullptr || shadowShader->GetID() != wantedShaderID)
			{
				if (shadowShader)
					shadowShader->Unbind();
				shadowShader = job.animationState ? shadowShaderSkeletal : shadowShaderStatic;
				shadowShader->BindWithCheck();
				shadowShader->SetUniform("lightSpaceMatrix", lightSpaceMatrix);
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
			debugInfo.drawCalls++;
		}

		shadowShader->Unbind();
		shadowFramebuffer->Unbind();
	}

	void Renderer::Draw()
	{
		SL_EVENT("Main Pass");

		viewportFramebuffer->Bind();

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


		const auto& batches = mainPass.GetBatches();
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
				Texture::BindTexture(environmentMap->GetIrradianceMapID(), 0, TextureTarget::TEXTURE_CUBE_MAP);
				Texture::BindTexture(environmentMap->GetPrefilterMapID(), 1, TextureTarget::TEXTURE_CUBE_MAP);
				Texture::BindTexture(environmentMap->GetBRDFTextureID(), 2, TextureTarget::TEXTURE_2D);
				Texture::BindTexture(shadowFramebuffer->GetDepthAttachmentID(), 3, TextureTarget::TEXTURE_2D);
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

			if (batch.animationID >= 0)
			{
				{
					SL_GPU_EVENT("Animation Texture");
					// Bind animation texture.
					Texture::BindTexture(batch.animationID, 16);
					currentShader->SetUniform("animTex", 16);
				}

				{
					SL_GPU_EVENT("Bone buffer");
					const size_t inverseBindPoseSize = SL_MAX_BONES * sizeof(Mat4);
					boneBuffer->SetData(batch.inverseBindPose, inverseBindPoseSize);
					boneBuffer->SetData(&batch.parents[0], SL_MAX_BONES * sizeof(int32_t) * 4, inverseBindPoseSize);
				}

				{
					SL_GPU_EVENT("Instance buffer");
					const size_t instanceDataSize = SL_MAX_INSTANCES * sizeof(Batch::InstanceData);
					instanceBuffer->SetData(batch.transforms.data(), batch.transforms.size() * sizeof(Mat4));
					instanceBuffer->SetData(batch.instances.data(), instanceDataSize, SL_MAX_INSTANCES * sizeof(Mat4));
				}
			}

			{
				SL_GPU_EVENT("Draw call");
				glDrawElementsInstanced(GL_TRIANGLES, batch.indexCount, GL_UNSIGNED_INT, 0, (GLsizei)batch.transforms.size());
			}

			debugInfo.drawCalls++;
		}

		environmentMap->Draw();

		viewportFramebuffer->Unbind();
	}

	void Renderer::EndScene()
	{
		SL_EVENT();
		glClear(GL_COLOR_BUFFER_BIT);

		screenShader->Bind();
		screenShader->SetUniform("screenTexture", 0);
		glBindVertexArray(Mesh::GetQuadVaoID());
		// glDisable(GL_DEPTH_TEST);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, viewportFramebuffer->GetColorAttachmentID(0));
		glDrawArrays(GL_TRIANGLES, 0, 6);
		screenShader->Unbind();
	}

	void Renderer::CleanUp()
	{
	}

	void Renderer::SetActiveCamera(Shared<Camera> inCamera, const Vec2& windowSize)
	{
		camera = inCamera;
		Resize(-windowSize.x / 2, -windowSize.y / 2, windowSize.x / 2, windowSize.y / 2);
	}

	RenderJob::RenderJob(unsigned int vaoID, unsigned int indexCount, Shared<Material> material, Shared<Shader> shader, const Mat4& transform) : vaoID(vaoID), indexCount(indexCount), material(material), shader(shader), transform(transform)
	{
	}

	RenderJob::RenderJob(unsigned int vaoID, unsigned int indexCount, Shared<Material> material, Shared<Shader> shader, const Mat4& transform, AnimationState* animationState) : vaoID(vaoID), indexCount(indexCount), material(material), shader(shader), transform(transform), animationState(animationState)
	{
	}

}
