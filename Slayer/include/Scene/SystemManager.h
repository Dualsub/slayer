#pragma once

#include "Core/Core.h"
#include "Core/Containers.h"
#include "Scene/System.h"

namespace Slayer {

    class SystemManager
    {
    private:
        Dict<SystemGroup, Vector<class System*>> m_systems;
    public:
        SystemManager();
        ~SystemManager();

        void Initialize();
        void Shutdown();

        void Update(float dt);
    };

}