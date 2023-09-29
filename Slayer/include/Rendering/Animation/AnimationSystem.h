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
			store.ForEach<Transform, SkeletalRenderer, AnimationPlayer>([&](Entity entity, Transform* transform, SkeletalRenderer* renderer, AnimationPlayer* player)
				{
					Shared<SkeletalModel> model = ResourceManager::Get()->GetAsset<SkeletalModel>(renderer->modelID);
					if (!model)
						return;
					AnimationState* state = &renderer->state;

					for (uint32_t i = 0; i < player->animationClips.size(); i++)
					{
						auto& clip = player->animationClips[i];
						Shared<Animation> animation = ResourceManager::Get()->GetAsset<Animation>(clip.animationID);

						float& time = clip.time;
						time = fmod(time + dt, animation->GetDuration());
						float weight = clip.weight;

						uint32_t frameNow = 0;
						uint32_t frameNext = 0;
						float timeFraction = 0.0f;

						const Vector<float>& times = animation->GetTimes();
						for (uint32_t j = 0; j < times.size(); j++)
						{
							if (time < times[j])
							{
								frameNow = j - 1;
								frameNext = j;
								timeFraction = (time - times[j - 1]) / (times[j] - times[j - 1]);
								break;
							}
						}

						state->SetAnimation(i, animation->GetTextureID(), timeFraction, { frameNow, frameNext }, weight);
					}
				});
		}

		void Render(Renderer& renderer, ComponentStore& store)
		{
			renderer.Skin();
		}
	};
}