#pragma once

#include "Core/Core.h"

#include <functional>

#define SL_SYSTEM(group) \
    friend class SystemManager; \
    constexpr SystemGroup GetGroup() const { return group; }

namespace Slayer
{
    enum SystemGroup : uint32_t
    {
        SL_GROUP_NONE = 0,
        SL_GROUP_INPUT = 1 << 0,
        SL_GROUP_AI = 1 << 1,
        SL_GROUP_PRE_PHYSICS = 1 << 2,
        SL_GROUP_FIXED_PHYSICS = 1 << 3,
        SL_GROUP_POST_PHYSICS = 1 << 4,
        SL_GROUP_TRANSFORM = 1 << 5,
        SL_GROUP_ANIMATION = 1 << 6,
        SL_GROUP_NETWORK = 1 << 7,
        SL_GROUP_AUDIO = 1 << 8,
        SL_GROUP_RENDER = 1 << 9,
    };


    class System
    {
    public:
        System() = default;
        virtual ~System() = default;

        virtual void Initialize() = 0;
        virtual void Shutdown() = 0;
        virtual void Update(Timespan dt, class ComponentStore& store) { SL_ASSERT(false && "Render function not implemented"); }
        virtual void Render(class Renderer& renderer, class ComponentStore& store) { SL_ASSERT(false && "Render function not implemented"); }
        virtual void OnActivated(class ComponentStore& store) { }
        virtual void OnDeactivated(class ComponentStore& store) { }
    };
}