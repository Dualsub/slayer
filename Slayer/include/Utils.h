#pragma once 

#include <iostream>
#include <fstream>
#include <filesystem>
#include <set>
#include <string>
#include <cstdint>

namespace Slayer {
    template <typename T>
    constexpr uint32_t HashType()
    {
        const char* typeName = typeid(T).name();
        uint32_t hash = 2166136261u;
        for (const char* p = typeName; *p != '\0'; ++p)
        {
            hash = (hash ^ static_cast<uint32_t>(*p)) * 16777619u;
        }
        return hash;
    }

    //std::string FindFileAbove(const std::string& root, const std::string& name, const uint32_t maxDepth = 5)
    //{
    //    // Search for the asset in the root directory
    //    std::string path = root + "/" + name;
    //    if (std::filesystem::exists(path))
    //        return path;

    //    std::set<std::string> visited;
    //    path = root;
    //    // Search for the asset in the parent directory
    //    for (uint32_t i = 0; i < maxDepth; i++)
    //    {
    //        path = "../" + path;
    //        // Serach in all subdirectories
    //        for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
    //        {
    //            if (visited.find(entry.path().string()) != visited.end())
    //                continue;
    //            if (entry.is_directory())
    //                continue;

    //            if (entry.path().filename() == name)
    //                return entry.path().string();
    //        }
    //    }

    //    return "";
    //}
}