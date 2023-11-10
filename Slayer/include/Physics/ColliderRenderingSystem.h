#pragma once

#include "Scene/System.h"

namespace Slayer {

    class ColliderRenderingSystem : public System<SystemGroup::SL_GROUP_RENDER>
    {
    public:
        ColliderRenderingSystem() = default;
        virtual ~ColliderRenderingSystem() = default;

        void Initialize() {}
        void Shutdown() {}
        void Update(float dt, class ComponentStore& store) {}

        void Render(class Renderer& renderer, class ComponentStore& store);
    };
}