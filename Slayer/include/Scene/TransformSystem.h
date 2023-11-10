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

		Mat4 GetWorldTransform(Entity entity, ComponentStore& store)
		{
			if (!store.IsValid(entity) || !store.HasComponent<Transform>(entity))
			{
				return Mat4(1.0f);
			}

			Transform* transform = store.GetComponent<Transform>(entity);

			Entity parentEntity = store.GetEntity(transform->parentId);
			if (parentEntity != SL_INVALID_ENTITY)
			{
				return GetWorldTransform(parentEntity, store) * transform->GetLocalMatrix();
			}
			else
			{
				return transform->GetLocalMatrix();
			}
		}

		virtual void Update(Timespan dt, ComponentStore& store) override
		{
			store.ForEach<Transform>([&](Entity entity, Transform* transform)
				{
					transform->worldTransform = GetWorldTransform(entity, store);
				});
		}

		virtual void Render(class Renderer& renderer, class ComponentStore& store) {}
	};

}