#pragma once

// The job system was based upon Molecular Musings' blog post and an implmentation by Jonathan Maldonado Contreras, which can bse found at:
// * http://molecularmusings.wordpress.com/2015/08/24/job-system-2-0-lock-free-work-stealing-part-1-basics/
// * https://github.com/Jonazan2/TinyJob

#include "Jobs/Job.h"

namespace Slayer
{
    class JobManager
    {
    friend class Engine;
    private:
        static JobManager *instance;
    public:
        JobManager() = default;
        ~JobManager() = default;

        bool Initialize();
        void Shutdown();

        void Push(Job* job);
        Job* Pop();
        Job* Steal();

        static JobManager *Get() { return instance; }
    };
}