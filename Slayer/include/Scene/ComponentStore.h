#pragma once

#include "Slayer.h"
#include "Utils.h"
#include "Serialization/Serialization.h"
#include "Resources/Asset.h"
#include "Scene/Components.h"

#include <future>

#define SL_MAX_ENTITIES 10000
#define SL_INVALID_ENTITY -1

namespace Slayer
{
    using Entity = uint32_t;
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
        size_t m_size = 0;

    public:

        virtual void RemoveEntity(Entity entity)
        {
            if (m_entityToIndexMap.find(entity) != m_entityToIndexMap.end())
            {
                m_emptyIndcies.insert(entity);
                m_entityToIndexMap.erase(entity);
            }
        }

        void InsertData(Entity entity, T component)
        {
            SL_ASSERT(m_entityToIndexMap.find(entity) == m_entityToIndexMap.end() && "Component added to same entity more than once.");
            SL_ASSERT(m_size < SL_MAX_ENTITIES && "Too many entities, consider increasing SL_MAX_ENTITIES.");

            if (m_emptyIndcies.empty())
            {
                m_componentArray.push_back(component);
                m_entityToIndexMap[entity] = m_size;
                m_size++;
            }
            else
            {
                size_t newIndex = *m_emptyIndcies.begin();
                m_emptyIndcies.erase(newIndex);
            }
        }

        T* GetComponent(Entity entity)
        {
            SL_ASSERT(m_entityToIndexMap.find(entity) != m_entityToIndexMap.end() && "Component not found for entity.");

            return &m_componentArray[m_entityToIndexMap[entity]];
        }
    };

    class SingletonComponent
    {
    public:
        SingletonComponent() = default;
        virtual ~SingletonComponent() = default;
        SingletonComponent(const SingletonComponent&) = delete;
        SingletonComponent& operator=(const SingletonComponent&) = delete;
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
        Entity entityIndex = 0;

        // Stores the what component types are associated with each entity
        DictHash<Entity, Archetype, EntityHash> m_entityComponentIndexMap;
        Dict<AssetID, Entity> m_entityIdMap;

        Dict<Archetype, std::set<Entity>> m_archetypeEntityMap;
        // Stores the component array for each component type
        ComponentDict m_components;
        SingletonDict m_singletons;

        inline void UpdateArachtype(Entity entity, Archetype oldArchetype, Archetype newArchetype)
        {
            if (m_archetypeEntityMap.find(oldArchetype) != m_archetypeEntityMap.end())
            {
                m_archetypeEntityMap[oldArchetype].erase(entity);
            }

            m_archetypeEntityMap[newArchetype].insert(entity);
            m_entityComponentIndexMap[entity] = newArchetype;
        }


    public:

        Entity CreateEntity()
        {
            SL_ASSERT(m_entityComponentIndexMap.size() < SL_MAX_ENTITIES && "Too many entities.");

            Entity entity = entityIndex++;

            EntityID component;
            component.id = GenerateAssetID();
            AddComponent(entity, component);

            return entity;
        }

        Entity CreateEntity(AssetID id)
        {
            SL_ASSERT(m_entityComponentIndexMap.size() < SL_MAX_ENTITIES && "Too many entities.");

            Entity entity = entityIndex++;

            EntityID component;
            component.id = id;
            AddComponent(entity, component);

            return entity;
        }

        Entity CreateEntityWithoutID()
        {
            SL_ASSERT(m_entityComponentIndexMap.size() < SL_MAX_ENTITIES && "Too many entities.");

            Entity entity = entityIndex++;

            return entity;
        }

        void DestroyEntity(Entity entity)
        {
            for (auto& [componentType, componentRecord] : m_components)
            {
                componentRecord.componentArray->RemoveEntity(entity);
            }

            m_entityComponentIndexMap.erase(entity);
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
            UpdateArachtype(entity, oldArch, newArch);
        }

        template <>
        void AddComponent(Entity entity, EntityID component)
        {
            // We overwrite the entity id if it already exists
            m_entityIdMap[component.id] = entity;
            GetComponentArray<EntityID>()->InsertData(entity, component);
            Archetype oldArch = m_entityComponentIndexMap[entity];
            Archetype newArch = oldArch | (uint64_t(1) << uint64_t(m_components[HashType<EntityID>()].bitIndex));
            UpdateArachtype(entity, oldArch, newArch);
        }

        template <typename C>
        void RemoveComponent(Entity entity)
        {
            GetComponentArray<C>()->RemoveEntity(entity);
            Archetype oldArch = m_entityComponentIndexMap[entity];
            Archetype newArch = oldArch & ~(1 << m_components[HashType<C>()].bitIndex);
            UpdateArachtype(entity, oldArch, newArch);
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

        template<typename T, typename... Args>
        void AddSingleton(Args... args)
        {
            uint32_t typeHash = HashType<T>();
            SL_ASSERT(m_singletons.find(typeHash) == m_singletons.end() && "Singleton already exists.");
            m_singletons.insert(std::make_pair(typeHash, MakeShared<T>(args)));
        }

        template<typename T>
        T* GetSingleton()
        {
            uint32_t typeHash = HashType<T>();
            SL_ASSERT(m_singletons.find(typeHash) != m_singletons.end() && "Singleton does not exist.");
            return static_cast<T*>(m_singletons[typeHash].get());
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

        template<typename Serializer>
        void Transfer(Serializer& serializer)
        {
            SL_ASSERT(serializer.PushObject("Scene"));
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
