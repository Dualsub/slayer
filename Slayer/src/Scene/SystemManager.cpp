#include "Scene/SystemManager.h"
#include "Scene/System.h"

namespace Slayer
{
    void SystemManager::Initialize()
    {
    }

    void SystemManager::Shutdown()
    {
    }

    void SystemManager::Update(SystemGroup group, float dt, class ComponentStore& store)
    {
        SystemCollection& collection = m_systemCollections[group];
        if (!collection.active)
        {
            return;
        }

        for (const Unique<System>& system : collection.systems)
        {
            system->Update(dt, store);
        }
    }

    void SystemManager::FixedUpdate(SystemGroup group, float dt, class ComponentStore& store)
    {
        SystemCollection& collection = m_systemCollections[group];
        if (!collection.active)
        {
            return;
        }

        for (const Unique<System>& system : collection.systems)
        {
            system->FixedUpdate(dt, store);
        }
    }

    void SystemManager::Render(SystemGroup group, class Renderer& renderer, class ComponentStore& store)
    {
        SystemCollection& collection = m_systemCollections[group];
        if (!collection.active)
        {
            return;
        }

        for (const Unique<System>& system : collection.systems)
        {
            system->Render(renderer, store);
        }
    }

    void SystemManager::ActivateAllGroups(class ComponentStore& store)
    {
        for (auto& pair : m_systemCollections)
        {
            SystemCollection& collection = pair.second;
            if (collection.active)
            {
                continue;
            }

            collection.active = true;
            for (const Unique<System>& system : collection.systems)
            {
                system->OnActivated(store);
            }
        }
    }

    void SystemManager::DeactivateAllGroups(class ComponentStore& store)
    {
        for (auto& pair : m_systemCollections)
        {
            SystemCollection& collection = pair.second;
            if (!collection.active)
            {
                continue;
            }

            collection.active = false;
            for (const Unique<System>& system : collection.systems)
            {
                system->OnDeactivated(store);
            }
        }
    }

    void SystemManager::ActivateSystemGroup(SystemGroup group, class ComponentStore& store)
    {
        if (group == SystemGroup::SL_GROUP_NONE && m_systemCollections.contains(group))
        {
            return;
        }

        SystemCollection& collection = m_systemCollections[group];
        if (collection.active)
        {
            return;
        }

        collection.active = true;
        for (const Unique<System>& system : collection.systems)
        {
            system->OnActivated(store);
        }
    }

    void SystemManager::DeactivateSystemGroup(SystemGroup group, class ComponentStore& store)
    {
        if (group == SystemGroup::SL_GROUP_NONE && m_systemCollections.contains(group))
        {
            return;
        }

        SystemCollection& collection = m_systemCollections[group];
        if (!collection.active)
        {
            return;
        }

        collection.active = false;
        for (const Unique<System>& system : collection.systems)
        {
            system->OnDeactivated(store);
        }
    }
}