#pragma once

#include "Scene/System.h"

namespace Slayer {

    class ContactSystem : public System
    {
        SL_SYSTEM(SystemGroup::SL_GROUP_PRE_PHYSICS)
    public:
        ContactSystem() = default;
        virtual ~ContactSystem() = default;

        void Initialize() {}
        void Shutdown() {}
        void Update(Timespan dt, class ComponentStore& store) {}
        void FixedUpdate(Timespan dt, class ComponentStore& store);

        void Render(class Renderer& renderer, class ComponentStore& store) {}
    };
}