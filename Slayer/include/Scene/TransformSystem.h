#pragma once

#include "Core/Core.h"
#include "Scene/System.h"
#include "Scene/ComponentStore.h"
#include "Scene/Components.h"
#include "Resources/ResourceManager.h"

namespace Slayer {

	class TransformSystem : public System
	{
		SL_SYSTEM(SystemGroup::SL_GROUP_TRANSFORM)
	public:
		virtual void Initialize() {};
		virtual void Shutdown() {};

		virtual void Update(Timespan dt, ComponentStore& store) override
		{
			store.ForEach<Transform>([&](Entity entity, Transform* transform)
				{
					Entity parentEntity = store.GetEntity(transform->parentId);
					if (store.IsValid(parentEntity) && store.HasComponent<Transform>(parentEntity))
					{
						if (store.HasComponent<SocketAttacher>(entity) && store.HasComponent<SkeletalSockets>(parentEntity))
						{
							auto* parentTransform = store.GetComponent<Transform>(parentEntity);
							auto* attacher = store.GetComponent<SocketAttacher>(entity);
							auto* sockets = store.GetComponent<SkeletalSockets>(parentEntity);
							transform->worldTransform = parentTransform->GetMatrix() * sockets->GetWorldTransform(attacher->name) * transform->GetMatrix();
						}
						else
						{
							transform->worldTransform = transform->GetMatrix();
						}
					}
					else
					{
						transform->worldTransform = transform->GetMatrix();
					}
				});
		}

		virtual void Render(class Renderer& renderer, class ComponentStore& store)
		{

		}
	};

}