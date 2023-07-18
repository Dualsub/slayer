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
					timeDict[entity] = (float)rand() / RAND_MAX * animation->GetDuration();
				}
				float& time = timeDict[entity];
				time = fmod(time + dt, animation->GetDuration());

				AnimationState* state = &renderer->state;
				
				uint32_t frameNow = 0;
				uint32_t frameNext = 0;
				float timeFraction = 0.0f;

				const Vector<float>& times = animation->GetTimes();
				for (uint32_t i = 0; i < times.size(); i++)
				{
					if (time < times[i])
					{
						frameNow = i - 1;
						frameNext = i;
						timeFraction = (time - times[i - 1]) / (times[i] - times[i - 1]);
						break;
					}
				}

				state->frames = { frameNow, frameNext };
				state->time = timeFraction;
				state->textureID = animation->GetTextureID();
			});
		}

		void Render(Renderer& renderer, ComponentStore& store) {}
	};
}