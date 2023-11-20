#include "Systems/FiringSystem.h"

#include "Resources/ResourceManager.h"
#include "Input/Input.h"

namespace Testbed {

    void FiringSystem::FixedUpdate(Slayer::Timespan dt, Slayer::ComponentStore& store)
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<float> dis(-1.0f, 1.0f);

        Slayer::Vector<BulletRequest> bulletRequests;

        store.ForEach<Slayer::Transform, BulletSource>([&](Slayer::Entity entity, Slayer::Transform* transform, BulletSource* source)
            {
                if (Slayer::Input::IsKeyPressed(Slayer::SlayerKey::KEY_F))
                {
                    BulletRequest request;
                    request.owner = entity;
                    request.timeToLive = source->bulletTimeToLive;
                    request.rotation = glm::normalize(glm::quat_cast(transform->worldTransform));
                    request.position = Slayer::Vec3(transform->worldTransform[3]) + (request.rotation * source->offset);

                    request.velocity = request.rotation * Slayer::Vec3(0.0f, -source->bulletSpeed, 0.0f);
                    float yawSpread = dis(gen) * source->spread;
                    float pitchSpread = dis(gen) * source->spread;
                    request.velocity = glm::rotate(glm::angleAxis(yawSpread, Slayer::Vec3(0.0f, 1.0f, 0.0f)), request.velocity);
                    request.velocity = glm::rotate(glm::angleAxis(pitchSpread, Slayer::Vec3(1.0f, 0.0f, 0.0f)), request.velocity);
                    bulletRequests.push_back(request);
                }
            }
        );

        // Not great, but it works
        auto* rm = Slayer::ResourceManager::Get();
        Slayer::AssetID bulletModel = rm->GetAssetID("unit_cube");
        Slayer::AssetID bulletMaterial = rm->GetAssetID("Mat_default");

        for (auto& request : bulletRequests)
        {
            Slayer::Entity bulletEntity = store.CreateEntity();

            store.AddComponent(bulletEntity, Slayer::Transform(request.position, request.rotation, Slayer::Vec3(10.0f, 10.0f, 10.0f)));

            auto rb = Slayer::RigidBody();
            rb.initialVelocity = request.velocity;
            rb.continuousCollision = true;
            rb.mass = 0.1f;
            store.AddComponent(bulletEntity, rb);

            store.AddComponent(bulletEntity, Slayer::ModelRenderer(bulletModel, { bulletMaterial }));
            store.AddComponent(bulletEntity, Slayer::BoxCollider(Slayer::Vec3(5.0f, 5.0f, 5.0f)));
            store.AddComponent(bulletEntity, Bullet(request.owner, request.timeToLive));
            store.AddComponent(bulletEntity, Slayer::CollisionListener());
        }

        store.ForEach<Bullet>([&](Slayer::Entity entity, Bullet* bullet)
            {
                if (bullet->lifeTime > bullet->timeToLive)
                {
                    store.DestroyEntityDeffered(entity);
                }

                bullet->lifeTime += dt;
            }
        );
    }

    void FiringSystem::Update(Slayer::Timespan dt, Slayer::ComponentStore& store)
    {

    }
}