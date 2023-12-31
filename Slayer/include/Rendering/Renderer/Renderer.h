#pragma once

#include "Core/Core.h"
#include "Core/Containers.h"
#include "Core/Math.h"
#include "Serialization/Serialization.h"
#include "Rendering/Renderer/Model.h"
#include "Rendering/Renderer/SkeletalModel.h"
#include "Rendering/Renderer/ComputeShader.h"
#include "Rendering/Renderer/Shader.h"
#include "Rendering/Renderer/Lights.h"
#include "Rendering/Renderer/Framebuffer.h"
#include "Rendering/Animation/AnimationState.h"

#define SL_MAX_INSTANCES 128
#define SL_MAX_SKELETONS 4
#define SL_MAX_ANIMATIONS 16

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

	struct AnimationBuffer
	{
		int32_t skeletonId = 0;
		int32_t padding[1] = { 0 };
		int32_t animationIds[SL_MAX_BLEND_ANIMATIONS] = { 0 };
		float weights[SL_MAX_BLEND_ANIMATIONS] = { 0.0f };
		float times[SL_MAX_BLEND_ANIMATIONS] = { 0.0f };
		Vec2i frames[SL_MAX_BLEND_ANIMATIONS] = { {0, 0} };


		AnimationBuffer() = default;
		~AnimationBuffer() = default;

		// Constructor for all values in the arrays
		AnimationBuffer(int32_t skeletonId, const int32_t* animationIds, const float* weights, const float* times, const Vec2i* frames)
			: skeletonId(skeletonId)
		{
			Copy(animationIds, this->animationIds, SL_MAX_BLEND_ANIMATIONS * sizeof(int32_t));
			Copy(weights, this->weights, SL_MAX_BLEND_ANIMATIONS * sizeof(float));
			Copy(times, this->times, SL_MAX_BLEND_ANIMATIONS * sizeof(float));
			Copy(frames, this->frames, SL_MAX_BLEND_ANIMATIONS * sizeof(Vec2i));
		}

	};

	struct Batch
	{
		unsigned int vaoID;
		unsigned int indexCount;

		Shared<Material> material;
		Shared<Shader> shader;

		Mat4* inverseBindPose;
		FixedVector<int32_t, SL_MAX_INSTANCES> animInstanceIds = {};

		FixedVector<Mat4, SL_MAX_INSTANCES> transforms = {};

		Batch(int32_t vaoID, int32_t indexCount, Shared<Material> material, Shared<Shader> shader, Mat4* inverseBindPose)
			: vaoID(vaoID), indexCount(indexCount), material(material), shader(shader), inverseBindPose(inverseBindPose)
		{
			std::memset(transforms.Data(), 0, sizeof(Mat4) * SL_MAX_BONES);
			std::memset(animInstanceIds.Data(), -1, sizeof(int32_t) * SL_MAX_INSTANCES);
		}

		void Add(const int32_t animInstanceId, const RenderJob& job)
		{
			transforms.PushBack(job.transform);
			animInstanceIds.PushBack(animInstanceId);
		}

		void Add(const RenderJob& job)
		{
			transforms.PushBack(job.transform);
		}

		static size_t GetHash(const RenderJob& job)
		{
			size_t hash = 0;
			const size_t prime = 31;

			hash = (hash * prime) ^ job.vaoID;
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
		Vector<AnimationState> animationStates;
		Shared<Framebuffer> framebuffer;
		SortingFunction sortingFunction;

		RenderPass() = default;
		RenderPass(Shared<Framebuffer> framebuffer, SortingFunction sortingFunction);

		void Submit(const RenderJob& job)
		{
			Batch* batch = nullptr;
			size_t hash = Batch::GetHash(job);
			// Create a new batch if it doesn't exist
			if (batchIndices.find(hash) == batchIndices.end())
			{
				batchIndices[hash] = batches.size();
				Batch newBatch(job.vaoID, job.indexCount, job.material, job.shader, job.animationState->inverseBindPose);
				batches.push_back(newBatch);
				batch = &batches.back();
			}
			else
			{
				batch = &batches[batchIndices[hash]];
			}

			if (job.animationState != nullptr)
			{
				animationStates.push_back(*job.animationState);
				batch->Add(animationStates.size() - 1, job);
			}
			else
			{
				batch->Add(job);
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

			if (animationStates.size() > 0)
			{
				animationStates.clear();
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

		Shared<Shader> m_screenShader;
		Shared<Framebuffer> m_viewportFramebuffer;
		Shared<Camera> m_camera;
		Shared<UniformBuffer> m_cameraBuffer;
		Shared<UniformBuffer> m_boneBuffer;
		Shared<UniformBuffer> m_instanceBuffer;
		Shared<UniformBuffer> m_lightsBuffer;

		// Lines
		Shared<VertexArray> m_lineVertexArray;
		Shared<VertexBuffer> m_lineVertexBuffer;
		Shared<Shader> m_lineShader;
		Vector<float> m_lineBuffer;

		Shared<Framebuffer> m_shadowFramebuffer;
		Shared<Shader> m_shadowShaderStatic;
		Shared<Shader> m_shadowShaderSkeletal;

		// Animation
		Shared<ComputeShader> m_animationShader;
		Shared<UniformBuffer> m_animationBuffer;
		Shared<Texture> m_boneTransformTexture;


		// Lights
		Mat4 m_lightProjection;
		Mat4 m_lightSpaceMatrix;
		Vec3 m_lightPos;

		// PBR
		Shared<Texture> m_hdrTexture;
		Shared<EnvironmentMap> m_environmentMap;
		Shared<Shader> m_shaderStatic;
		Shared<Shader> m_shaderSkeletal;

		DirectionalLight m_directionalLight;
		Vector<PointLight> m_pointLights;
		LightInfo m_lightInfo;
		ShadowInfo m_shadowInfo;
		RenderPass m_shadowPass;
		RenderPass m_mainPass;
		DebugInfo m_debugInfo;

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
		void Submit(Shared<SkeletalModel> model, const Mat4& transform, AnimationState* animationState);
		void Submit(Shared<SkeletalModel> model, AnimationState* animationState, Shared<Material> material, const Mat4& transform);
		void Submit(Shared<Model> model, Shared<Material> material, const Mat4& transform);
		void Submit(Shared<SkeletalModel> model, Shared<Material> material, const Mat4& transform);
		void Submit(Shared<Mesh> mesh, const Mat4& transform);
		void SubmitQuad(Shared<Material> material, const Mat4& transform);
		void SubmitLine(Vec3 p1, Vec3 p2, Vec4 color);
		void Skin();
		void DrawShadows();
		void DrawLines();
		void Draw();
		void EndScene();
		void CleanUp();

		// Debug
		DebugInfo& GetDebugInfo() { return m_debugInfo; }
	};

}