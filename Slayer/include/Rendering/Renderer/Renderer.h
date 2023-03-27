#pragma once

#include "Core/Core.h"
#include "Core/Containers.h"
#include "Core/Math.h"
#include "Serialization/Serialization.h"
#include "Rendering/Renderer/Model.h"
#include "Rendering/Renderer/SkeletalModel.h"
#include "Rendering/Renderer/Lights.h"
#include "Rendering/Renderer/Framebuffer.h"

namespace Slayer {

	class Shader;
	class Camera;
	class EnvironmentMap;

	struct LightInfo
	{
		//Vector<PointLight> pointLights;
		DirectionalLight directionalLight = DirectionalLight(Vec3(-1.0f), Vec3(1.0f));
		template<typename Serializer>
		void Transfer(Serializer& serializer)
		{
			SL_TRANSFER_VAR(directionalLight);
		}
	};

	struct ShadowInfo
	{
		Vec3 lightPos = Vec3(0.0);
		float width = 4 * 2048;
		float height = 4 * 2048;
		float near = 10.0f;
		float far = 1000.0f;
		float distance = -30.0f;

		template<typename Serializer>
		void Transfer(Serializer& serializer)
		{
			SL_TRANSFER_VAR(distance);
			SL_TRANSFER_VAR(lightPos);
			SL_TRANSFER_VAR(width);
			SL_TRANSFER_VAR(height);
			SL_TRANSFER_VAR(near);
			SL_TRANSFER_VAR(far);
		}
	};

	struct RenderJob
	{
		unsigned int vaoID;
		unsigned int indexCount;
		Shared<Material> material;
		Shared<Shader> shader;
		Mat4 transform;
		Mat4* boneMatrices = nullptr;
		RenderJob(unsigned int vaoID, unsigned int indexCount, Shared<Material> material, Shared<Shader> shader, const Mat4& transform);
		RenderJob(unsigned int vaoID, unsigned int indexCount, Shared<Material> material, Shared<Shader> shader, const Mat4& transform, Mat4* boneMatrices);
	};

	using SortingFunction = std::function<bool(const RenderJob&, const RenderJob&)>;

	struct RenderPass
	{
		Vector<RenderJob> queue;
		Shared<Framebuffer> framebuffer;
		SortingFunction sortingFunction;

		RenderPass() = default;
		RenderPass(Shared<Framebuffer> framebuffer, SortingFunction sortingFunction);
		void Submit(const RenderJob& job)
		{
			queue.push_back(job);
		}

		void Sort()
		{
			std::sort(queue.begin(), queue.end(), sortingFunction);
		}

		void Clear()
		{
			queue.clear();
		}

		const Vector<RenderJob>& GetQueue() const
		{
			return queue;
		}
	};

	struct LineJob
	{

	};

	struct DebugInfo
	{
		int drawCalls = 0;
	};

	struct BoneData
	{
		Mat4 boneMatrices[MAX_BONES];
		BoneData(Mat4* inBoneMatrices)
		{
			std::copy(inBoneMatrices, (Mat4*)(inBoneMatrices + (MAX_BONES - 1) * sizeof(Mat4)), boneMatrices);
		}
	};

	struct CameraData
	{
		Mat4 projectionMatrix;
		Mat4 viewMatrix;
		Vec3 position;
		float padding;
		CameraData(const Mat4& projectionMatrix, const Mat4& viewMatrix, const Vec3& position)
			: projectionMatrix(projectionMatrix), viewMatrix(viewMatrix), position(position)
		{
		}
	};

	class Renderer
	{
	private:
		const int MAX_LINES = 10000;

		Shared<Shader> screenShader;
		Shared<Framebuffer> viewportFramebuffer;
		Shared<Camera> camera;
		Shared<UniformBuffer> cameraBuffer;
		Shared<UniformBuffer> boneBuffer;
		Shared<UniformBuffer> lightsBuffer;

		// Lines
		Shared<VertexArray> lineVertexArray;
		Shared<VertexBuffer> lineVertexBuffer;
		Shared<Shader> lineShader;
		Vector<float> lineBuffer;

		Shared<Framebuffer> shadowFramebuffer;
		Shared<Shader> shadowShaderStatic;
		Shared<Shader> shadowShaderSkeletal;

		// Lights
		Mat4 lightProjection;
		Mat4 lightSpaceMatrix;
		Vec3 lightPos;

		// PBR
		Shared<Texture> hdrTexture;
		Shared<EnvironmentMap> environmentMap;
		Shared<Shader> shaderStatic;
		Shared<Shader> shaderSkeletal;

		DirectionalLight directionalLight;
		Vector<PointLight> pointLights;
		LightInfo lightInfo;
		ShadowInfo shadowInfo;
		RenderPass shadowPass;
		RenderPass mainPass;
		DebugInfo debugInfo;

		void BindMaterial(Shared<Material> material, Shared<Shader> shader);
	public:
		void SetActiveCamera(Shared<Camera> inCamera, const Vec2& windowSize);
		void Initialize(Shared<Camera> inCamera, int width, int height);
		void Resize(int width, int height);
		void Resize(int x, int y, int width, int height);
		void Clear();
		void BeginScene();
		void BeginScene(const LightInfo& lightInfo, const ShadowInfo& shadowSettings);
		void BeginScene(const Vector<PointLight>& inPointLights, const DirectionalLight& inDirectionalLight);
		void Submit(Shared<SkeletalModel> model, const Mat4& transform, Mat4* inBoneMatrices);
		void Submit(Shared<Model> model, Shared<Material> material, const Mat4& transform);
		void Submit(Shared<Mesh> mesh, const Mat4& transform);
		void SubmitQuad(Shared<Material> material, const Mat4& transform);
		void SubmitLine(Vec3 p1, Vec3 p2, Vec4 color);
		void DrawShadows();
		void DrawLines();
		void Draw();
		void EndScene();
		void CleanUp();

		// Debug
		DebugInfo& GetDebugInfo() { return debugInfo; }
	};

}