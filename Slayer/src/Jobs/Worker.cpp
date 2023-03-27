#include "Jobs/Job.h"
#include "Jobs/Worker.h"
#include "Jobs/ThreadManager.h"

namespace Slayer
{
    Worker::Worker(std::size_t maxJobs, Mode mode)
        : thread{nullptr}, mode{mode}, state{State::RUNNING}, jobPool{maxJobs}
    {
        workQueue = MakeUnique<JobQueue>(maxJobs);
    }

    Worker::~Worker()
    {
        state = State::IDLE;
        if(thread != nullptr)
        {
            thread->join();
            delete thread;
        }

        workQueue.reset();
    }

    void Worker::StartBackgroundWorker()
    {
        state = State::RUNNING;
        thread = new std::thread{&Worker::Run, this};
        threadId = thread->get_id();
    }

    void Worker::Run()
    {
        while (IsRunning())
        {
            Job *job = GetJob();

            if (job != nullptr)
            {
                job->Run();
            }
        }
    }

    void Worker::Submit(Job *job)
    {
        workQueue->Push(job);
    }

    void Worker::Wait(Job *waitJob)
    {
        while (!waitJob->Finished())
        {
            Job *job = GetJob();

            if (job != nullptr)
            {
                job->Run();
            }
        }
    }

    Job *Worker::GetJob()
    {
        Job *job = workQueue->Pop();

        if (job != nullptr)
        {
            job->Run();
        }
        else
        {
            Worker *worker = ThreadManager::Get()->GetRandomWorker();

            if (worker != this)
            {
                Job *job = worker->workQueue->Steal();

                if (job != nullptr)
                {
                    return job;
                }
                else
                {
                    std::this_thread::yield();
                    return nullptr;
                }
            }
            else
            {
                std::this_thread::yield();
                return nullptr;
            }
        }

        return job;
    }
}