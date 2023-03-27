#pragma once

#include "Core/Core.h"
#include "Scene/ComponentStore.h"

namespace Slayer
{
    struct SceneHeader
    {
        uint32_t nameLength;
        uint32_t entityCount;
    };

    struct EntityHeader
    {
        uint32_t componentCount;
    };

    struct ComponentHeader
    {
        uint32_t type;
        uint32_t size;
    };

    class SceneSerializer
    {
    private:
        std::vector<char> m_data;
    public:
        SceneSerializer() = default;
        ~SceneSerializer() = default;

        void Serialize(ComponentStore& store, const std::vector<char>& data)
        {
            // SceneHeader header;
            // header.nameLength = "ScaneName";
            // header.entityCount = store.GetEntityCount();

            // Copy(data.data(), &header, sizeof(SceneHeader));

            // uint32_t offset = sizeof(SceneHeader);
            // Copy(data.data() + offset, "Scene", header.nameLength);
            // offset += header.nameLength;

            // for (const auto& entity : store.GetAllEntities())
            // {
            //     EntityHeader entityHeader;
            //     entityHeader.componentCount = store.GetComponentCount(entity);
            //     Copy(data.data() + offset, &entityHeader, sizeof(EntityHeader));
            //     offset += sizeof(EntityHeader);

            //     for (const auto& [type, record] : store.GetComponents(entity))
            //     {
            //         ComponentHeader componentHeader;
            //         componentHeader.type = type;
            //         componentHeader.size = record.size;
            //         Copy(data.data() + offset, &componentHeader, sizeof(ComponentHeader));
            //         offset += sizeof(ComponentHeader);
            //         Copy(data.data() + offset, component.second.data(), component.second.size());
            //         offset += record.size;
            //     }
            // }
        }

        // template<typename T>
        // void Transfer(const std::string& name, T& data)
        // {
        //     // The unknown type T is serialized by calling the Serialize function
        //     data.Serialize(this);
        // }

        // template<>
        // void Transfer(const std::string& name, float& data)
        // {
        //     // Push all bytes of float into vector
        //     Copy((char*)&data, (char*)&data + sizeof(float), std::back_inserter(m_data));
        // }

        // template<>
        // void Transfer(const std::string& name, uint32_t& data)
        // {
        //     // Push all bytes of float into vector
        //     Copy((char*)&data, (char*)&data + sizeof(uint32_t), std::back_inserter(m_data));
        // }

        // template<>
        // void Transfer(const std::string& name, std::string& data)
        // {
        //     // First push the length of the string
        //     uint32_t length = data.length();
        //     Copy((char*)&length, (char*)&length + sizeof(uint32_t), std::back_inserter(m_data));
        //     // Push all bytes of float into vector
        //     Copy(data.data(), data.data() + length, std::back_inserter(m_data));
        // }
    };

    class SceneDeserializer
    {
    public:
        bool Deserialize(ComponentStore& store, const std::vector<char>& data);

        // template<typename T>
        // void Transfer(const std::string& name, T& data)
        // {
        //     data.Serialize(this);
        // }

        // template<>
        // void Transfer(const std::string& name, float& data)
        // {
        //     Copy(m_data.data(), &data, sizeof(float));
        // }

        // template<>
        // void Transfer(const std::string& name, uint32_t& data)
        // {
        //     Copy(m_data.data(), &data, sizeof(uint32_t));
        // }

        // template<>
        // void Transfer(const std::string& name, std::string& data)
        // {
        //     uint32_t length;
        //     Copy(m_data.data(), &length, sizeof(uint32_t));
        //     for (uint32_t i = 0; i < length; i++)
        //     {
        //         data.push_back(m_data[i]);
        //     }
        // }
    };
}