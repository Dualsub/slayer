#pragma once

#include "Scene/System.h"
#include "Scene/ComponentStore.h"
#include "Scene/Components.h"
#include "Resources/ResourceManager.h"

#include "Rendering/Renderer/Renderer.h"

namespace Slayer {

    class RenderingSystem : public System<SystemGroup::SL_GROUP_RENDER>
    {
    public:
        RenderingSystem() = default;
        virtual ~RenderingSystem() = default;

        void Initialize()
        {

        }

        void Shutdown()
        {

        }

        void Update(float dt, ComponentStore& store)
        {
        }
        
        void Render(Renderer& renderer, ComponentStore& store)
        {
            ResourceManager* rm = ResourceManager::Get();
            store.ForEach<Transform, SkeletalRenderer>([&](Entity entity, Transform* transform, SkeletalRenderer* modelRenderer)
            {
                Shared<SkeletalModel> model = rm->GetAsset<SkeletalModel>(modelRenderer->modelID);
                Shared<Material> material = rm->GetAsset<Material>(modelRenderer->materialID);
                renderer.Submit(model, modelRenderer->boneTransforms, material, transform->worldTransform);
            });
        }
    };
}