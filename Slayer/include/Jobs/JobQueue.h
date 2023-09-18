#pragma once

#include "Core/Core.h"
#include "Jobs/Job.h"
#include <atomic>

namespace Slayer
{
    class JobQueue
    {
    private:
    public:
        JobQueue(std::size_t maxJobs);
        JobQueue();
        ~JobQueue() = default;

        void Push(Job* job);
        Job* Pop();
        Job* Steal();
        void Clear() { m_bottom = m_top = 0; }
        size_t Size() const { return m_bottom - m_top; }

    private:
        std::atomic_int m_bottom;
        std::atomic_int m_top;
        Vector<Job*> m_jobs;
    };
}