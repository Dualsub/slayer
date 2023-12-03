#pragma once

#include "Slayer.h"
#include "Utils.h"
#include "Serialization/Serialization.h"
#include "Resources/Asset.h"
#include "Scene/Entity.h"
#include "Scene/Components.h"
#include "Scene/SingletonComponent.h"


#include <future>

#define SL_MAX_ENTITIES 10000

namespace Slayer
{
    using ComponentType = uint32_t;
    using Archetype = uint64_t;

    struct EntityHash
    {
        size_t operator()(const Entity& e) const noexcept
        {
            return static_cast<size_t>(e);
        }
    };

    template <typename T>
    std::string GetSanitizedTypeName()
    {
        std::string typeName = std::string(typeid(T).name());
        size_t lastColon = typeName.find_last_of(':');
        if (lastColon != std::string::npos)
        {
            typeName = typeName.substr(lastColon + 1);
        }
        return typeName;
    }

    class ComponentArrayBase
    {
    public:
        virtual void RemoveEntity(Entity entity) = 0;

        ComponentArrayBase() = default;
        virtual ~ComponentArrayBase() = default;
    };

    template <typename T>
    class ComponentArray : public ComponentArrayBase
    {
    private:
        std::vector<T> m_componentArray;
        std::unordered_map<Entity, size_t, EntityHash> m_entityToIndexMap;
        // Vacant entities are stored in a set
        std::set<size_t> m_emptyIndcies;
    public:

        virtual void RemoveEntity(Entity entity) override
        {
            if (m_entityToIndexMap.find(entity) != m_entityToIndexMap.end())
            {
                size_t index = m_entityToIndexMap[entity];
                m_emptyIndcies.insert(index);
                m_entityToIndexMap.erase(entity);
            }
        }

        void InsertData(Entity entity, T component)
        {
            SL_ASSERT(m_entityToIndexMap.find(entity) == m_entityToIndexMap.end() && "Component added to same entity more than once.");
            SL_ASSERT(m_componentArray.size() < SL_MAX_ENTITIES && "Too many entities, consider increasing SL_MAX_ENTITIES.");

            if (m_emptyIndcies.empty())
            {
                m_componentArray.push_back(component);
                m_entityToIndexMap[entity] = m_componentArray.size() - 1;
            }
            else
            {
                size_t newIndex = *m_emptyIndcies.begin();
                m_emptyIndcies.erase(newIndex);
                m_componentArray[newIndex] = component;
                m_entityToIndexMap[entity] = newIndex;
            }
        }

        T* GetComponent(Entity entity)
        {
            SL_ASSERT(m_entityToIndexMap.find(entity) != m_entityToIndexMap.end() && "Component not found for entity.");

            return &m_componentArray[m_entityToIndexMap[entity]];
        }
    };


    struct ArchetypeTransition
    {
        Entity entity;
        Archetype oldArchetype;
        Archetype newArchetype;
    };

    class ComponentStore
    {
    public:
        struct ComponentRecord
        {
            ComponentType type = 0;
            size_t size = 0;
            uint32_t bitIndex = 0;
            Shared<ComponentArrayBase> componentArray;
        };

        using ComponentDict = DictHash<ComponentType, ComponentRecord, std::hash<ComponentType>>;
        using SingletonDict = DictHash<ComponentType, Shared<SingletonComponent>, std::hash<ComponentType>>;
    private:
        Entity m_entityIndex = 0;

        // Stores the what component types are associated with each entity
        DictHash<Entity, Archetype, EntityHash> m_entityComponentIndexMap;
        Dict<AssetID, Entity> m_entityIdMap;

        Dict<Archetype, std::set<Entity>> m_archetypeEntityMap;
        // Stores the component array for each component type
        ComponentDict m_components;
        SingletonDict m_singletons;

        // Stores the transition of entities from one archetype to another
        Vector<ArchetypeTransition> m_archetypeTransitions;
        Vector<Entity> entityFreeList;

        inline void UpdateArchetype(Entity entity, Archetype oldArchetype, Archetype newArchetype)
        {
            if (m_archetypeEntityMap.find(oldArchetype) != m_archetypeEntityMap.end())
            {
                m_archetypeEntityMap[oldArchetype].erase(entity);
            }

            m_archetypeEntityMap[newArchetype].insert(entity);
            m_entityComponentIndexMap[entity] = newArchetype;
            m_archetypeTransitions.push_back({ entity, oldArchetype, newArchetype });
        }


    public:

        Entity CreateEntity()
        {
            SL_ASSERT(m_entityComponentIndexMap.size() < SL_MAX_ENTITIES && "Too many entities.");

            Entity entity = m_entityIndex++;

            EntityID component;
            component.id = GenerateAssetID();
            AddComponent(entity, component);

            return entity;
        }

        Entity CreateEntity(AssetID id)
        {
            SL_ASSERT(m_entityComponentIndexMap.size() < SL_MAX_ENTITIES && "Too many entities.");

            Entity entity = m_entityIndex++;

            EntityID component;
            component.id = id;
            AddComponent(entity, component);

            return entity;
        }

        Entity CreateEntityWithoutID()
        {
            SL_ASSERT(m_entityComponentIndexMap.size() < SL_MAX_ENTITIES && "Too many entities.");

            Entity entity = m_entityIndex++;

            return entity;
        }

        void DestroyEntity(Entity entity)
        {
            if (HasComponent<EntityID>(entity))
            {
                EntityID* id = GetComponent<EntityID>(entity);
                m_entityIdMap.erase(id->id);
            }

            for (auto& [componentType, componentRecord] : m_components)
            {
                componentRecord.componentArray->RemoveEntity(entity);
            }

            const Archetype archetype = m_entityComponentIndexMap[entity];
            m_archetypeEntityMap[archetype].erase(entity);
            m_entityComponentIndexMap.erase(entity);
        }

        // Deletes the entity at the end of the frame
        void DestroyEntityDeffered(Entity entity)
        {
            entityFreeList.push_back(entity);

            const Archetype archetype = m_entityComponentIndexMap[entity];
            m_archetypeTransitions.push_back({ entity, archetype, 0 });
        }

        // Deletes all entities, should be called at the end of the frame
        void DestroyDefferedEntities()
        {
            for (auto& entity : entityFreeList)
            {
                DestroyEntity(entity);
            }

            entityFreeList.clear();
        }

        // TODO: Optimize to add components directly to the new entity
        Entity DuplicateEntity(Entity entity)
        {
            Entity newEntity = CreateEntity();

            ForEachComponentType([this, entity, newEntity]<typename T>()
            {
                if (!HasComponent<T>(entity) || HashType<T>() == HashType<EntityID>() || HashType<T>() == HashType<TagComponent>())
                    return;

                T* component = GetComponent<T>(entity);
                AddComponent<T>(newEntity, *component);
            });

            return newEntity;
        }

        bool IsValid(Entity entity)
        {
            return entity != SL_INVALID_ENTITY && m_entityComponentIndexMap.find(entity) != m_entityComponentIndexMap.end();
        }

        Entity GetEntity(AssetID id)
        {
            if (m_entityIdMap.find(id) != m_entityIdMap.end())
            {
                return m_entityIdMap[id];
            }

            return SL_INVALID_ENTITY;
        }

        template <typename... Ts>
        Entity FindFirst()
        {
            Archetype archetype = GetArchetype<Ts...>();

            if (m_archetypeEntityMap.find(archetype) != m_archetypeEntityMap.end())
            {
                return *m_archetypeEntityMap[archetype].begin();
            }

            return SL_INVALID_ENTITY;
        }

        template <typename... Ts>
        void RegisterComponents()
        {
            (RegisterComponent<Ts>(), ...);
        }

        template <typename T>
        void RegisterComponent()
        {
            const ComponentType typeHash = HashType<T>();

            SL_ASSERT(m_components.find(typeHash) == m_components.end() && "Registering component type more than once.");

            m_components[typeHash] = { typeHash, sizeof(T), (uint32_t)m_components.size(), std::make_unique<ComponentArray<T>>() };
        }

        template <typename T>
        ComponentArray<T>* GetComponentArray()
        {
            uint32_t typeHash = HashType<T>();

            if (m_components.find(typeHash) == m_components.end())
                RegisterComponent<T>();

            return static_cast<ComponentArray<T> *>(m_components[typeHash].componentArray.get());
        }

        template <typename C>
        void AddComponent(Entity entity, C component)
        {
            GetComponentArray<C>()->InsertData(entity, component);
            Archetype oldArch = m_entityComponentIndexMap[entity];
            Archetype newArch = oldArch | (uint64_t(1) << uint64_t(m_components[HashType<C>()].bitIndex));
            UpdateArchetype(entity, oldArch, newArch);
        }

        template <>
        void AddComponent(Entity entity, EntityID component)
        {
            // We overwrite the entity id if it already exists
            m_entityIdMap[component.id] = entity;
            GetComponentArray<EntityID>()->InsertData(entity, component);
            Archetype oldArch = m_entityComponentIndexMap[entity];
            Archetype newArch = oldArch | (uint64_t(1) << uint64_t(m_components[HashType<EntityID>()].bitIndex));
            UpdateArchetype(entity, oldArch, newArch);
        }

        template <typename C>
        void RemoveComponent(Entity entity)
        {
            GetComponentArray<C>()->RemoveEntity(entity);
            Archetype oldArch = m_entityComponentIndexMap[entity];
            Archetype newArch = oldArch & ~(1 << m_components[HashType<C>()].bitIndex);
            UpdateArchetype(entity, oldArch, newArch);
        }

        template <typename T>
        T* GetComponent(Entity entity)
        {
            return GetComponentArray<T>()->GetComponent(entity);
        }

        template <typename T>
        bool HasComponent(Entity entity)
        {
            return (m_entityComponentIndexMap[entity] & (uint64_t(1) << uint64_t(m_components[HashType<T>()].bitIndex))) != 0;
        }

        template <typename... Ts>
        void ForEach(auto&& func)
        {
            const std::vector<uint32_t> hashes = { HashType<Ts>()... };

            // Build a bit mask of all the component types
            Archetype searchArchetype = GetArchetype<Ts...>();

            for (auto& [archetype, entitySet] : m_archetypeEntityMap)
            {
                if ((archetype & searchArchetype) == searchArchetype)
                {
                    for (auto& entity : entitySet)
                    {
                        func(entity, GetComponent<Ts>(entity)...);
                    }
                }
            }
        }

        template <typename... Ts>
        void ForEachAsync(auto&& func)
        {
            const std::vector<uint32_t> hashes = { HashType<Ts>()... };

            // Build a bit mask of all the component types
            Archetype searchArchetype = 0;
            for (auto& hash : hashes)
            {
                SL_ASSERT(m_components.find(hash) != m_components.end() && "Component not registered before use.");
                searchArchetype |= (1 << m_components[hash].bitIndex);
            }

            std::vector<std::future<void>> futures;

            for (auto& [archetype, entitySet] : m_archetypeEntityMap)
            {
                if ((archetype & searchArchetype) == searchArchetype)
                {
                    for (auto& entity : entitySet)
                    {
                        futures.push_back(std::async(std::launch::async, func, entity, GetComponent<Ts>(entity)...));
                    }
                }
            }

            for (auto& future : futures)
            {
                future.get();
            }
        }

        template <typename... Ts>
        std::vector<Entity> GetEntities(bool excludeArchetypesWithMoreComponents = false) const
        {
            const std::vector<uint32_t> hashes = { HashType<Ts>()... };

            // Build a bit mask of all the component types
            Archetype searchArchetype = 0;
            for (auto& hash : hashes)
            {
                SL_ASSERT(m_components.find(hash) != m_components.end() && "Component not registered before use.");
                searchArchetype |= (1 << m_components[hash].bitIndex);
            }

            std::vector<Entity> entities;

            if (!excludeArchetypesWithMoreComponents)
            {
                for (auto& [archetype, entitySet] : m_archetypeEntityMap)
                {
                    if ((archetype & searchArchetype) == searchArchetype)
                    {
                        entities.reserve(entities.size() + entitySet.size());
                        for (auto& entity : entitySet)
                        {
                            entities.emplace_back(entity);
                        }
                    }
                }
            }
            else
            {
                if (m_archetypeEntityMap.find(searchArchetype) != m_archetypeEntityMap.end())
                {
                    entities.reserve(m_archetypeEntityMap[searchArchetype].size());
                    for (auto& entity : m_archetypeEntityMap[searchArchetype])
                    {
                        entities.emplace_back(entity);
                    }
                }
            }

            return entities;
        }

        std::vector<Entity> GetAllEntities() const
        {
            std::vector<Entity> entities;
            entities.reserve(m_entityComponentIndexMap.size());
            for (auto& [entity, archetype] : m_entityComponentIndexMap)
            {
                entities.emplace_back(entity);
            }

            return entities;
        }

        size_t GetEntityCount() const
        {
            return m_entityComponentIndexMap.size();
        }

        template <typename... Ts>
        Archetype GetArchetype()
        {
            const std::vector<uint32_t> hashes = { HashType<Ts>()... };

            // Build a bit mask of all the component types
            Archetype searchArchetype = 0;
            for (auto& hash : hashes)
            {
                SL_ASSERT(m_components.find(hash) != m_components.end() && "Component not registered before use.");
                searchArchetype |= (1 << m_components[hash].bitIndex);
            }

            return searchArchetype;
        }

        size_t GetComponentCount(Entity entity)
        {
            Archetype archetype = m_entityComponentIndexMap[entity];
            // Count the number of bits set in the archetype
            size_t count = 0;
            while (archetype)
            {
                count += archetype & 1;
                archetype >>= 1;
            }

            return count;
        }

        const ComponentDict& GetComponents()
        {
            return m_components;
        }

        template<typename T>
        void AddSingleton(const T& singleton)
        {
            uint32_t typeHash = HashType<T>();
            SL_ASSERT(m_singletons.find(typeHash) == m_singletons.end() && "Singleton already exists.");
            m_singletons.emplace(typeHash, MakeShared<T>(singleton));
        }

        template<typename T>
        T* GetSingleton()
        {
            uint32_t typeHash = HashType<T>();
            SL_ASSERT(m_singletons.find(typeHash) != m_singletons.end() && "Singleton does not exist.");
            return static_cast<T*>(m_singletons[typeHash].get());
        }

        template<typename T, typename Func>
        void WithSingleton(Func&& func)
        {
            uint32_t typeHash = HashType<T>();
            if (m_singletons.find(typeHash) == m_singletons.end())
                return;
            func(static_cast<T*>(m_singletons[typeHash].get()));
        }

        template<typename T>
        void RemoveSingleton()
        {
            uint32_t typeHash = HashType<T>();
            if (m_singletons.find(typeHash) != m_singletons.end())
            {
                m_singletons.erase(typeHash);
            }
        }

        template<typename... Ts>
        void ForTransitionTo(auto&& func)
        {
            const Archetype archetype = GetArchetype<Ts...>();
            for (auto& transition : m_archetypeTransitions)
            {
                if ((archetype & transition.newArchetype) == archetype && (archetype & transition.oldArchetype) != archetype)
                {
                    func(transition.entity, GetComponent<Ts>(transition.entity)...);
                }
            }
        }

        // DISCLAMER: The entity could be deleted before the function is called.
        template<typename... Ts>
        void ForTransitionFrom(auto&& func)
        {
            const Archetype archetype = GetArchetype<Ts...>();
            for (auto& transition : m_archetypeTransitions)
            {
                if ((archetype & transition.oldArchetype) == archetype && (archetype & transition.newArchetype) != archetype)
                {
                    func(transition.entity, GetComponent<Ts>(transition.entity)...);
                }
            }
        }

        void ClearTransitions()
        {
            m_archetypeTransitions.clear();
        }

        // Debug
        const Vector<Entity>& GetEntityFreeList()
        {
            return entityFreeList;
        }

        const Vector<ArchetypeTransition>& GetArchetypeTransitions()
        {
            return m_archetypeTransitions;
        }

        template<typename Serializer>
        void Transfer(Serializer& serializer)
        {
            SL_ASSERT(serializer.PushObject("Scene"));

            if (serializer.PushObject("Singletons"))
            {
                if (serializer.GetFlags() == SerializationFlags::Read)
                {
                    ForEachSingletonType([this, &serializer]<typename T>()
                    {
                        WithSingleton<T>([this, &serializer]<typename U>(U * singleton)
                        {
                            std::string typeName = GetSanitizedTypeName<T>();
                            serializer.Transfer(*singleton, typeName);
                        });
                    });
                }
                else if (serializer.GetFlags() == SerializationFlags::Write)
                {
                    ForEachSingletonType([this, &serializer]<typename T>()
                    {
                        std::string typeName = GetSanitizedTypeName<T>();
                        if (!serializer.IsValid(typeName))
                            return;

                        T singleton;
                        serializer.Transfer(singleton, typeName);
                        AddSingleton<T>(singleton);
                    });
                }

                serializer.PopObject();
            }


            SL_ASSERT(serializer.PushArray("Entities"));

            if (serializer.GetFlags() == SerializationFlags::Read)
            {
                for (auto& [entity, archetype] : m_entityComponentIndexMap)
                {
                    serializer.PushObject();
                    ForEachComponentType([this, &serializer, entity]<typename T>()
                    {
                        if (!HasComponent<T>(entity))
                            return;

                        std::string typeName = GetSanitizedTypeName<T>();
                        T* component = GetComponent<T>(entity);
                        serializer.Transfer(*component, typeName);
                    });
                    serializer.PopObject();
                }
            }
            else if (serializer.GetFlags() == SerializationFlags::Write)
            {
                while (serializer.Next())
                {
                    Entity entity = CreateEntityWithoutID();
                    serializer.PushArrayElement();
                    ForEachComponentType([this, &serializer, entity]<typename T>()
                    {
                        std::string typeName = GetSanitizedTypeName<T>();
                        if (!serializer.IsValid(typeName))
                            return;
                        T component;
                        serializer.Transfer(component, typeName);
                        AddComponent<T>(entity, component);
                    });
                    serializer.PopArrayElement();
                }
            }

            serializer.PopArray();
            serializer.PopObject();
        }
    };

}
