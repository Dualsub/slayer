#include "jobs/ThreadManager.h"
#include <random>

namespace Slayer
{
    ThreadManager* ThreadManager::instance = nullptr;

    bool ThreadManager::Initialize()
    {
        static const int jobsPerThread = 4;
        static const int workerThreads = 8;
        std::size_t jobsPerQueue = jobsPerThread;
        workers.emplace_back(MakeUnique<Worker>(jobsPerThread, Worker::Mode::FOREGROUND));

        for (std::size_t i = 1; i < workerThreads; ++i)
        {
            workers.emplace_back(MakeUnique<Worker>(jobsPerThread, Worker::Mode::BACKGROUND));
        }

        for (auto& worker : workers)
        {
            worker->Run();
        }

        return true;
    }

    void ThreadManager::Shutdown()
    {
        for (auto& worker : workers)
        {
            worker.release();
        }

        workers.clear();
    }

    Job* ThreadManager::CreateJob(JobFunction function)
    {
        return GetRandomWorker()->Pool().CreateJob(function);
    }

    Worker* ThreadManager::GetRandomWorker()
    {
        static std::random_device rd;
        static std::uniform_int_distribution<size_t> dist(0, workers.size() - 1);
        return workers[dist(rd)].get();
    }

    Worker* ThreadManager::FindThreadWorker(const std::thread::id threadId)
    {
        for (auto& worker : workers)
        {
            if (worker->GetThreadId() == threadId)
            {
                return worker.get();
            }
        }

        return nullptr;
    }

}