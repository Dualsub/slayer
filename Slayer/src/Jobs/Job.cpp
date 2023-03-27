#include "Jobs/Job.h"

namespace Slayer
{
    Job::Job(JobFunction function, Job *parent)
        : function{function}, parent{parent}, unfinishedJobs{1}
    {
        if (parent != nullptr)
        {
            parent->unfinishedJobs++;
        }
    }

    void Job::Run()
    {
        function(*this);
        Finish();
    }

    bool Job::Finished() const
    {
        return unfinishedJobs == 0;
    }

    void Job::Finish()
    {
        unfinishedJobs--;

        if (Finished())
        {
            if (parent != nullptr)
            {
                parent->Finish();
            }
        }
    }
}