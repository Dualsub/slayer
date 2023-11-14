#pragma once

#include "Scene/System.h"

namespace Slayer {

    class PhysicsSystem : public System
    {
        SL_SYSTEM(SystemGroup::SL_GROUP_FIXED_PHYSICS)
    public:
        PhysicsSystem() = default;
        virtual ~PhysicsSystem() = default;

        void Initialize() {}
        void Shutdown() {}
        void Update(float dt, class ComponentStore& store);

        void Render(class Renderer& renderer, class ComponentStore& store) {}
    };
}