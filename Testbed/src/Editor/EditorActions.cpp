#include "Editor/EditorActions.h"

#include "Serialization/YamlSerializer.h"

#include <future>

namespace Slayer::Editor::Actions {

    void SaveScene(ComponentStore& store, const std::string& path)
    {
        std::thread thread([path, &store]() {
            YamlSerializer serializer;
            serializer.Serialize(store, path);
            Log::Info("Saving scene to:", path);
            });
        thread.detach();
    }

    std::future<void> LoadScene(ComponentStore& store, const std::string& path)
    {
        return std::async(std::launch::async, [path, &store]() {
            YamlDeserializer deserializer;
            deserializer.Deserialize(store, path);
            });
    }
}