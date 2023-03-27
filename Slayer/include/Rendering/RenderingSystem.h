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
            store.ForEach<Transform>([&](Entity entity, Transform* transform)
            {
                transform->rotation *= Quat(Vec3(0.0f, 0.0f, dt * 0.5f));
            });
        }
        
        void Render(Renderer& renderer, ComponentStore& store)
        {
            ResourceManager* rm = ResourceManager::Get();
            store.ForEach<Transform, ModelRenderer>([&](Entity entity, Transform* transform, ModelRenderer* modelRenderer)
            {
                Mat4 transformMatrix = transform->GetMatrix();
                Shared<Model> model = rm->GetAsset<Model>(modelRenderer->modelID);
                Shared<Material> material = rm->GetAsset<Material>(modelRenderer->materialID);
                renderer.Submit(model, material, transformMatrix);
            });
        }
    };
}