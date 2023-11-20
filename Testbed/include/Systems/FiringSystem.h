#pragma once

#include "Core/Core.h"
#include "Scene/System.h"
#include "Scene/ComponentStore.h"

namespace Testbed {

    struct BulletRequest
    {
        Slayer::Entity owner = SL_INVALID_ENTITY;
        float timeToLive = 5.0f;
        Slayer::Vec3 position;
        Slayer::Quat rotation;
        Slayer::Vec3 velocity;
    };

    class FiringSystem : public Slayer::System
    {
    public:
        SL_SYSTEM(Slayer::SystemGroup::SL_GROUP_PRE_PHYSICS)

            FiringSystem() = default;
        virtual ~FiringSystem() = default;

        virtual void Initialize() {};
        virtual void Shutdown() {};
        virtual void Update(Slayer::Timespan dt, Slayer::ComponentStore& store) override;
        virtual void FixedUpdate(Slayer::Timespan dt, Slayer::ComponentStore& store) override;
        virtual void Render(class Renderer& renderer, class ComponentStore& store) {}
    };
}
