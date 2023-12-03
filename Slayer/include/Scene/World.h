#pragma once 

#include "Core/Singleton.h"
#include "Scene/Components.h"
#include "Scene/ComponentStore.h"
#include "Physics/PhysicsWorld.h"

namespace Slayer {

    class World : public Singleton<World>
    {
    private:
        ComponentStore m_store;
        PhysicsWorld m_physicsWorld;
    public:
        World()
        {
            SetInstance(this);
            m_physicsWorld.Initialize();
        }

        ~World()
        {
            m_physicsWorld.Shutdown();
            SetInstance(nullptr);
        }

        static ComponentStore& GetWorldStore() { SL_ASSERT(s_instance); return s_instance->m_store; }
        ComponentStore& GetStore() { return m_store; }
        void SetStore(ComponentStore& store, bool clearPhysics = true)
        {
            if (clearPhysics)
                m_physicsWorld.RemoveAllRigidBodies();
            m_store = store;
        }

        static PhysicsWorld& GetPhysicsWorld() { SL_ASSERT(s_instance); return s_instance->m_physicsWorld; }
        PhysicsWorld& GetWorld() { return m_physicsWorld; }
    };
}



