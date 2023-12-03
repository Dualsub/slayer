#pragma once

#include "Core/Core.h"
#include "Core/Containers.h"
#include "Core/Singleton.h"
#include "Scene/System.h"

namespace Slayer
{
    class SystemManager : public Singleton<SystemManager>
    {
    private:
        struct SystemCollection
        {
            bool active = false;
            Vector<Unique<class System>> systems;
        };

        Dict<SystemGroup, SystemCollection> m_systemCollections;

    public:
        SystemManager()
        {
            SetInstance(this);
        }

        ~SystemManager()
        {
            SetInstance(nullptr);
        }

        void Initialize();
        void Shutdown();

        void Update(SystemGroup group, float dt, class ComponentStore& store);
        void FixedUpdate(SystemGroup group, float dt, class ComponentStore& store);
        void Render(SystemGroup group, class Renderer& renderer, class ComponentStore& store);

        template <typename T, typename... Args>
        void RegisterSystem(Args &&...args)
        {
            Unique<System> system = MakeUnique<T>(std::forward<Args>(args)...);
            SystemGroup group = static_cast<T*>(system.get())->GetGroup();
            m_systemCollections[group].systems.push_back(std::move(system));
        }

        void ActivateAllGroups(class ComponentStore& store);
        void DeactivateAllGroups(class ComponentStore& store);
        void ToggleSystemGroup(bool activate, SystemGroup group, class ComponentStore& store);
        void ActivateSystemGroup(SystemGroup group, class ComponentStore& store);
        void DeactivateSystemGroup(SystemGroup group, class ComponentStore& store);
    };

}