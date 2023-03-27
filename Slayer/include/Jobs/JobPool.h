#pragma once

#include "Jobs/Job.h"
#include <vector>

namespace Slayer
{
    class JobPool
    {
    public:
        JobPool(std::size_t maxJobs);

        Job *Allocate();
        bool Full() const;
        void Clear();

        Job *CreateJob(JobFunction jobFunction);
        Job *CreateJobAsChild(JobFunction& jobFunction, Job *parent);

        template <typename Data>
        Job *CreateJob(JobFunction jobFunction, const Data &data);
        template <typename Data>
        Job *CreateJobAsChild(JobFunction jobFunction, const Data &data, Job *parent);

    private:
        std::size_t allocatedJobs;
        std::vector<Job> storage;
    };
}