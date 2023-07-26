#pragma once

#include "Core/Core.h"
#include "Core/Containers.h"
#include "Core/Math.h"
#include "Serialization/Serialization.h"
#include "Rendering/Renderer/Model.h"
#include "Rendering/Renderer/SkeletalModel.h"
#include "Rendering/Renderer/Shader.h"
#include "Rendering/Renderer/Lights.h"
#include "Rendering/Renderer/Framebuffer.h"
#include "Rendering/Animation/AnimationState.h"

#define SL_MAX_INSTANCES 300

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
		AnimationState* animationState = nullptr;
		RenderJob(unsigned int vaoID, unsigned int indexCount, Shared<Material> material, Shared<Shader> shader, const Mat4& transform);
		RenderJob(unsigned int vaoID, unsigned int indexCount, Shared<Material> material, Shared<Shader> shader, const Mat4& transform, AnimationState* animationState);
	};

	using SortingFunction = std::function<bool(const RenderJob&, const RenderJob&)>;

	struct Batch
	{
		struct InstanceData
		{
			Vec2i frames;
			float time;
			float padding = 0.0;

			InstanceData(const Vec2i& frames, float time)
				: frames(frames), time(time)
			{
			}
		};

		unsigned int vaoID;
		unsigned int indexCount;
		unsigned int animationID;
		Shared<Material> material;
		Shared<Shader> shader;
		int32_t parents[SL_MAX_BONES * 4];
		Mat4* inverseBindPose;
		Vector<Mat4> transforms = {};
		Vector<InstanceData> instances = {};

		Batch(const RenderJob& job)
			: vaoID(job.vaoID), indexCount(job.indexCount), animationID(job.animationState->textureID), material(job.material), shader(job.shader), inverseBindPose(job.animationState->inverseBindPose)
		{
			for (int i = 0; i < SL_MAX_BONES; i++)
			{
				parents[i * 4] = job.animationState->parents[i];
			}
			transforms.reserve(SL_MAX_INSTANCES);
			instances.reserve(SL_MAX_INSTANCES);

			std::memset(transforms.data(), 0, sizeof(Mat4) * SL_MAX_INSTANCES);
			std::memset(instances.data(), 0, sizeof(InstanceData) * SL_MAX_INSTANCES);

			Add(job);
		}

		void Add(const RenderJob& job)
		{
			transforms.push_back(job.transform);
			instances.push_back(InstanceData(job.animationState->frames, job.animationState->time));
		}

		// Generate batch hash without external functions
		static size_t GetHash(const RenderJob& job)
		{
			size_t hash = 0;
			const size_t prime = 31; // A common prime number used for hash calculations

			hash = (hash * prime) ^ job.vaoID;
			hash = (hash * prime) ^ job.animationState->textureID;
			hash = (hash * prime) ^ job.shader->GetID();
			hash = (hash * prime) ^ job.material->assetID;

			return hash;
		}
	};

	struct RenderPass
	{
		Vector<RenderJob> queue;
		Dict<size_t, size_t> batchIndices;
		Vector<Batch> batches;
		Shared<Framebuffer> framebuffer;
		SortingFunction sortingFunction;

		RenderPass() = default;
		RenderPass(Shared<Framebuffer> framebuffer, SortingFunction sortingFunction);

		void Submit(const RenderJob& job)
		{
			if (job.animationState == nullptr)
				return;

			// queue.push_back(job);
			size_t hash = Batch::GetHash(job);
			if (batchIndices.find(hash) == batchIndices.end())
			{
				batchIndices[hash] = batches.size();
				batches.push_back(Batch(job));
			}
			else
			{
				batches[batchIndices[hash]].Add(job);
			}
		}

		void Sort()
		{
			std::sort(queue.begin(), queue.end(), sortingFunction);
		}

		void Clear()
		{
			if (queue.size() > 0)
			{
				queue.clear();
			}

			if (batches.size() > 0)
			{
				batches.clear();
			}

			if (batchIndices.size() > 0)
			{
				batchIndices.clear();
			}
		}

		const Vector<RenderJob>& GetQueue() const
		{
			return queue;
		}

		const Vector<Batch>& GetBatches() const
		{
			return batches;
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
		Mat4 boneMatrices[SL_MAX_BONES];
		BoneData(Mat4* inBoneMatrices)
		{
			std::copy(inBoneMatrices, (Mat4*)(inBoneMatrices + (SL_MAX_BONES - 1) * sizeof(Mat4)), boneMatrices);
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
		Shared<UniformBuffer> instanceBuffer;
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
		void BindAnimation(AnimationState* animationState, Shared<Shader> shader);
	public:
		void SetActiveCamera(Shared<Camera> inCamera, const Vec2& windowSize);
		void Initialize(Shared<Camera> inCamera, int width, int height);
		void Resize(int width, int height);
		void Resize(int x, int y, int width, int height);
		void Clear();
		void BeginScene();
		void BeginScene(const LightInfo& lightInfo, const ShadowInfo& shadowSettings);
		void BeginScene(const Vector<PointLight>& inPointLights, const DirectionalLight& inDirectionalLight);
		void Submit(Shared<SkeletalModel> model, const Mat4& transform, AnimationState* animationState);
		void Submit(Shared<SkeletalModel> model, AnimationState* animationState, Shared<Material> material, const Mat4& transform);
		void Submit(Shared<Model> model, Shared<Material> material, const Mat4& transform);
		void Submit(Shared<SkeletalModel> model, Shared<Material> material, const Mat4& transform);
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