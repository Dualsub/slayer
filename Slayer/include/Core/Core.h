#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <array>
#include <cassert>
#include <unordered_map>
#include <map>
#include <set>
#include <stack>
#include <cstdint>
#include <typeinfo>
#include <initializer_list>
#include <functional>
#include <tuple>

#undef NDEBUG
#define SL_ASSERT(x) assert(x)

#define SL_EVENT(x)

namespace Slayer
{
    template<typename T>
    using Unique = std::unique_ptr<T>;
    template<typename T, typename ...Args>
    Unique<T> MakeUnique(Args... args)
    {
        return std::move(std::make_unique<T>(args...));
    }
    template<typename T>
    using Shared = std::shared_ptr<T>;
    template<typename T, typename ...Args>
    Shared<T> MakeShared(Args... args)
    {
        return std::move(std::make_shared<T>(args...));
    }

    template<typename T, typename U>
    void Copy(T* src, U* dst, size_t size)
    {
        memcpy(dst, src, size);
    }

    using Timespan = float;
}