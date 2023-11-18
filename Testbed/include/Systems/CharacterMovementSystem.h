#pragma once

#include "Core/Core.h"
#include "Scene/System.h"

namespace Testbed {

    class CharacterMovementSystem : public Slayer::System
    {
    public:
        SL_SYSTEM(Slayer::SystemGroup::SL_GROUP_PRE_PHYSICS)

            CharacterMovementSystem() = default;
        virtual ~CharacterMovementSystem() = default;

        virtual void Initialize() {};
        virtual void Shutdown() {};
        virtual void Update(Slayer::Timespan dt, Slayer::ComponentStore& store) override;
        virtual void FixedUpdate(Slayer::Timespan dt, Slayer::ComponentStore& store) override;
        virtual void Render(class Renderer& renderer, class ComponentStore& store) {}
    };
}
