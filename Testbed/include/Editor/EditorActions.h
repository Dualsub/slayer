#pragma once

#include "Core/Core.h"
#include "Scene/ComponentStore.h"

namespace Slayer::Editor::Actions {
    void SaveScene(ComponentStore& store, const std::string& path);
    std::future<void> LoadScene(ComponentStore& store, const std::string& path);
}