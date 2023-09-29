#pragma once 

#include "Core/Singleton.h"
#include "Scene/Components.h"
#include "Scene/ComponentStore.h"

namespace Slayer {

    class World : public Singleton<World>
    {
    private:
        ComponentStore m_store;
    public:
        World()
        {
            SetInstance(this);
        }
        ~World()
        {
            SetInstance(nullptr);
        }

        static ComponentStore& GetWorldStore() { return s_instance->m_store; }
        ComponentStore& GetStore() { return m_store; }
        void SetStore(ComponentStore& store) { m_store = store; }
    };
}



