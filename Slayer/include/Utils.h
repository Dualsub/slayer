#pragma once 

#include <iostream>

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
}