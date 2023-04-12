#pragma once

#include "Core/Core.h"

namespace Slayer {

    enum SystemGroup : uint32_t
    {
        SL_GROUP_NONE = 0,
        SL_GROUP_RENDER = 1 << 0,
        SL_GROUP_PHYSICS = 1 << 1,
        SL_GROUP_AI = 1 << 2,
        SL_GROUP_AUDIO = 1 << 3,
        SL_GROUP_TRANSFORM = 1 << 4,
        SL_GROUP_NETWORK = 1 << 5,
        SL_GROUP_ANIMATION = 1 << 6,
    };

    template <SystemGroup ...Groups>
    class System
    {
    private:
        uint32_t m_groupFlags = (Groups | ...);
    public:
        System() = default;
        virtual ~System() = default;

        constexpr uint32_t GetGroupFlags() const { return m_groupFlags; }
        bool IsInGroup(SystemGroup group) const { return m_groupFlags & group; }

        virtual void Initialize() = 0;
        virtual void Shutdown() = 0;
        virtual void Update(Timespan dt, class ComponentStore& store) { SL_ASSERT(false && "Render function not implemented"); }
        virtual void Render(class Renderer& renderer, class ComponentStore& store) { SL_ASSERT(false && "Render function not implemented"); }
    };

}