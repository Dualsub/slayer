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
			Mat4 localMatrix = transform->GetLocalMatrix();

			Entity parentEntity = store.GetEntity(transform->parentId);
			if (parentEntity != SL_INVALID_ENTITY)
			{
				Mat4 parentMatrix = GetWorldTransform(parentEntity, store);
				if (store.HasComponent<SocketAttacher>(entity) && store.HasComponent<SkeletalRenderer>(parentEntity))
				{
					SocketAttacher* socketAttacher = store.GetComponent<SocketAttacher>(entity);
					SkeletalRenderer* renderer = store.GetComponent<SkeletalRenderer>(parentEntity);
					Shared<SkeletalModel> model = ResourceManager::Get()->GetAsset<SkeletalModel>(renderer->modelID);
					int32_t boneId = model->GetBoneID(socketAttacher->name);
					Mat4 boneMarix = Renderer::Get()->GetBoneTransform(0, boneId);
					parentMatrix *= boneMarix;
				}

				return parentMatrix * localMatrix;
			}
			else
			{
				return localMatrix;
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