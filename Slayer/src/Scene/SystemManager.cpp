#include "Scene/SystemManager.h"
#include "Scene/System.h"

namespace Slayer
{

    SystemManager::SystemManager()
    {
    }

    SystemManager::~SystemManager()
    {
    }

    void SystemManager::Initialize()
    {
    }

    void SystemManager::Shutdown()
    {
    }

    void SystemManager::Update(SystemGroup group, float dt, class ComponentStore &store)
    {
        SystemCollection &collection = m_systemCollections[group];
        if (!collection.active)
        {
            return;
        }

        for (const Unique<System> &system : collection.systems)
        {
            system->Update(dt, store);
        }
    }

    void SystemManager::Render(SystemGroup group, class Renderer &renderer, class ComponentStore &store)
    {
        SystemCollection &collection = m_systemCollections[group];
        if (!collection.active)
        {
            return;
        }

        for (const Unique<System> &system : collection.systems)
        {
            system->Render(renderer, store);
        }
    }

    template <typename T, typename... Args>
    void SystemManager::RegisterSystem(Args &&...args)
    {
        Unique<System> system = MakeUnique<T>(std::forward<Args>(args)...);
        SystemGroup group = system->GetGroupFlags();
        m_systemCollections[group].systems.push_back(std::move(system));
    }

    void SystemManager::ActivateAllGroups(class ComponentStore &store)
    {
        for (auto &pair : m_systemCollections)
        {
            SystemCollection &collection = pair.second;
            if (collection.active)
            {
                continue;
            }

            collection.active = true;
            for (const Unique<System> &system : collection.systems)
            {
                system->OnActivated(store);
            }
        }
    }

    void SystemManager::DeactivateAllGroups(class ComponentStore &store)
    {
        for (auto &pair : m_systemCollections)
        {
            SystemCollection &collection = pair.second;
            if (!collection.active)
            {
                continue;
            }

            collection.active = false;
            for (const Unique<System> &system : collection.systems)
            {
                system->OnDeactivated(store);
            }
        }
    }

    void SystemManager::ActivateSystemGroup(SystemGroup group, class ComponentStore &store)
    {
        SystemCollection &collection = m_systemCollections[group];
        if (collection.active)
        {
            return;
        }

        collection.active = true;
        for (const Unique<System> &system : collection.systems)
        {
            system->OnActivated(store);
        }
    }

    void SystemManager::DeactivateSystemGroup(SystemGroup group, class ComponentStore &store)
    {
        SystemCollection &collection = m_systemCollections[group];
        if (!collection.active)
        {
            return;
        }

        collection.active = false;
        for (const Unique<System> &system : collection.systems)
        {
            system->OnDeactivated(store);
        }
    }
}