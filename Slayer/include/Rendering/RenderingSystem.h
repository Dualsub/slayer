#pragma once

#include "Scene/System.h"
#include "Scene/ComponentStore.h"
#include "Scene/Components.h"
#include "Resources/ResourceManager.h"

#include "Rendering/Renderer/Renderer.h"

namespace Slayer {

    class RenderingSystem : public System
    {
        SL_SYSTEM(SystemGroup::SL_GROUP_RENDER)
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
            SL_EVENT();

            store.WithSingleton<WorldRenderingSettings>([&](WorldRenderingSettings* settings)
                {
                    renderer.SetGamma(settings->gamma);
                    renderer.SetExposure(settings->exposure);
                    renderer.SetShadowCascadeIndex(settings->shadowCascadeIndex);
                });

            store.WithSingleton<WorldCamera>([&store, &renderer](WorldCamera* camera)
                {
                    Entity attachEntity = store.GetEntity(camera->attachEntityId);
                    if (!store.IsValid(attachEntity) || !store.HasComponent<Transform>(attachEntity))
                        return;

                    Transform* transform = store.GetComponent<Transform>(attachEntity);

                    Application* app = Application::Get();
                    float width = (float)app->GetWindow().GetWidth();
                    float height = (float)app->GetWindow().GetHeight();
                    Mat4 projection = glm::perspective(glm::radians(camera->fov), width / height, camera->nearPlane, camera->farPlane);

                    Mat4 view = glm::inverse(transform->GetMatrix());

                    renderer.SetCameraData(
                        camera->nearPlane,
                        camera->farPlane,
                        camera->fov,
                        width / height,
                        projection,
                        view,
                        transform->position);
                });

            store.WithSingleton<DirectionalLight>([&](DirectionalLight* light)
                {
                    renderer.SetDirectionalLight(light->orientation, light->intensity * light->color);

                });

            ResourceManager* rm = ResourceManager::Get();
            store.ForEach<Transform, SkeletalRenderer>([&](Entity entity, Transform* transform, SkeletalRenderer* modelRenderer)
                {
                    Shared<SkeletalModel> model = rm->GetAsset<SkeletalModel>(modelRenderer->modelID);
                    Shared<Material> material = rm->GetAsset<Material>(modelRenderer->materialID);
                    if (!model || !material)
                        return;

                    modelRenderer->state.inverseBindPose = model->GetInverseBindPoseMatrices();
                    modelRenderer->state.parents = model->GetParents();
                    renderer.Submit(model, &modelRenderer->state, material, transform->worldTransform);
                });

            store.ForEach<Transform, ModelRenderer>([&](Entity entity, Transform* transform, ModelRenderer* modelRenderer)
                {
                    Shared<Model> model = rm->GetAsset<Model>(modelRenderer->modelID);
                    Vector<Shared<Material>> materials;
                    for (auto& materialAsset : modelRenderer->materialIDs)
                    {
                        Shared<Material> material = rm->GetAsset<Material>(materialAsset.id);
                        materials.push_back(material);
                    }

                    if (!model)
                        return;

                    renderer.Submit(model, materials, transform->worldTransform);
                });
        }
    };
}