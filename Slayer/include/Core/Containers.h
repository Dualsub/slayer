#include "Core/Core.h"

// We wrap the STL containers in our own namespace to avoid conflicts with other libraries 
// and to make it easier to change the underlying implementation.

namespace Slayer 
{
    template<typename T>
    using Vector = std::vector<T>;
    template<typename T, typename U>
    using Dict = std::unordered_map<T, U>;
    template<typename T, typename U, typename Hash>
    using DictHash = std::unordered_map<T, U, Hash>;
    template<typename T, typename U>
    using Map = std::map<T, U>;
    template<typename T>
    using Set = std::set<T>;
    template<typename T, typename U>
    using Tuple = std::tuple<T, U>;
}