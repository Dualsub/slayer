#pragma once

#include "Scene/System.h"

namespace Slayer {

    class ColliderRenderingSystem : public System
    {
        SL_SYSTEM(SystemGroup::SL_GROUP_DEBUG_RENDER)
    public:
        ColliderRenderingSystem() = default;
        virtual ~ColliderRenderingSystem() = default;

        void Initialize() {}
        void Shutdown() {}

        void Render(class Renderer& renderer, class ComponentStore& store);
    };
}