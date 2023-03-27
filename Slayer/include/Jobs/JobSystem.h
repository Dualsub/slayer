#pragma once
#include <functional>

namespace Slayer
{

    struct JobDispatchArgs
    {
        uint32_t jobIndex;
        uint32_t groupIndex;
    };

    namespace JobSystem
    {
        void Initialize();
        void Execute(const std::function<void()>& job);
        void Dispatch(uint32_t jobCount, uint32_t groupSize, const std::function<void(JobDispatchArgs)>& job);
        bool IsBusy();
        void Wait();
    }

} // namespace Slayer
