#include "Jobs/JobPool.h"
#include <assert.h>

namespace Slayer
{
    JobPool::JobPool(std::size_t maxJobs)
        : allocatedJobs{0}, storage{maxJobs}
    {
        assert(storage.size() == maxJobs);
    }

    Job *JobPool::Allocate()
    {
        if (!Full())
        {
            return &storage[allocatedJobs++];
        }
        else
        {
            return nullptr;
        }
    }

    void JobPool::Clear()
    {
        allocatedJobs = 0;
    }

    bool JobPool::Full() const
    {
        return allocatedJobs == storage.size();
    }

    Job *JobPool::CreateJob(JobFunction jobFunction)
    {
        Job *job = Allocate();

        if (job != nullptr)
        {
            new (job) Job{jobFunction, nullptr};
            return job;
        }
        else
        {
            return nullptr;
        }
    }

}