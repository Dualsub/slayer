#include "Serialization/SceneSerializer.h"
#include "Scene/Components.h"

namespace Slayer 
{

    bool SceneDeserializer::Deserialize(ComponentStore& store, const std::vector<char>& data)
    {
        SceneHeader header;
        Copy(&header, data.data(), sizeof(SceneHeader));

        uint32_t offset = sizeof(SceneHeader);
        std::string name;
        for (uint32_t i = 0; i < header.nameLength; i++)
        {
            name.push_back(data[offset + i]);
        }

        offset += header.nameLength;

        for (uint32_t i = 0; i < header.entityCount; i++)
        {
            Entity entity = store.CreateEntity();
            EntityHeader entityHeader;
            Copy(&entityHeader, data.data() + offset, sizeof(EntityHeader));
            offset += sizeof(EntityHeader);

            ForEachComponentType([&]<typename T>()
            {
                if (store.HasComponent<T>(entity))
                {
                    ComponentHeader componentHeader;
                    Copy(&componentHeader, data.data() + offset, sizeof(ComponentHeader));
                    offset += sizeof(ComponentHeader);

                    m_data = std::vector<char>(data.begin() + offset, data.begin() + offset + componentHeader.size);
                    T& component = store.GetComponent<T>(entity);
                    component.Serialize(this);
                    offset += componentHeader.size;
                }
            });
        }
    }
}