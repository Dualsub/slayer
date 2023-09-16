#pragma once

#undef NDEBUG
#undef NRELEASE

#include <iostream>
#include <vector>
#include <memory>
#include <array>
#include <assert.h>
#include <unordered_map>
#include <string>
#include <map>
#include <set>
#include <stack>
#include <cstdint>
#include <typeinfo>
#include <initializer_list>
#include <functional>
#include <tuple>
#include <algorithm>

#define SL_ASSERT(x) assert(x)

#define PROFILING_ENABLED 1
// Profiling
#if PROFILING_ENABLED
#include "optick.h"
#define SL_FRAME(ARGS) OPTICK_FRAME(ARGS)
#define SL_EVENT(...) OPTICK_EVENT(__VA_ARGS__)
#define SL_GPU_EVENT(...) OPTICK_GPU_EVENT(__VA_ARGS__)
#else
#define SL_FRAME(VA_ARGS)
#define SL_EVENT(...)
#endif // PROFILING_ENABLED


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