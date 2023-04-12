#include "Scene/System.h"
#include "Scene/ComponentStore.h"
#include "Scene/Components.h"
#include "Resources/ResourceManager.h"

#include "Rendering/Renderer/Renderer.h"

namespace Slayer {

	class AnimationSystem : public System<SystemGroup::SL_GROUP_ANIMATION>
	{
	public:
		AnimationSystem() = default;
		virtual ~AnimationSystem() = default;

		void Initialize()
		{

		}

		void Shutdown()
		{

		}

		void Update(float dt, ComponentStore& store)
		{
			store.ForEach< Transform, SkeletalRenderer>([&](Entity entity, Transform* transform, SkeletalRenderer* renderer)
			{
				Shared<SkeletalModel> model = ResourceManager::Get()->GetAsset<SkeletalModel>(renderer->modelID);
				Shared<Animation> animation = ResourceManager::Get()->GetAsset<Animation>(9414454467584602516);

				// Rotate random bone
				static Dict<Entity, float> timeDict;
				if (timeDict.find(entity) == timeDict.end())
				{
					timeDict[entity] = 0.0f;
				}
				float& time = timeDict[entity];
				time = fmod(time + dt * animation->GetTicksPerSecond(), animation->GetDuration());

				auto& bones = model->GetBones();
				auto globalInv = model->GetGlobalInverseTransform();

				Vector<Mat4> localTransform(bones.size());
				Vector<Mat4> modelTransform(bones.size());

				localTransform[0] = animation->SampleChannel(bones[0].name, time);
				modelTransform[0] = localTransform[0];
				renderer->boneTransforms[0] = globalInv * modelTransform[0] * bones[0].offset;
				SkeletalSockets* sockets = store.HasComponent<SkeletalSockets>(entity) ? store.GetComponent<SkeletalSockets>(entity) : nullptr;

				for (size_t i = 1; i < bones.size(); ++i)
				{
					auto& bone = bones[i];
					localTransform[i] = animation->SampleChannel(bone.name, time);
					modelTransform[i] = modelTransform[bone.parentID] * localTransform[i];
					renderer->boneTransforms[i] = globalInv * modelTransform[i] * bone.offset;
				}
			});
		}

		void Render(Renderer& renderer, ComponentStore& store) {}
	};
}