#pragma once 

#include "Scene/Components.h"
#include "Scene/ComponentStore.h"

namespace Slayer {

    class Scene
    {
    private:
        ComponentStore componentStore;
    public:
        Scene() = default;
        ~Scene() = default;
    };
}



