#pragma once

#include "Slayer.h"
#include "Jobs/Worker.h"

namespace Slayer
{
    class ThreadManager
    {
    private:
        static ThreadManager* instance;
        std::vector<std::unique_ptr<Worker>> workers;
    public:
        ThreadManager() = default;
        ~ThreadManager() = default;

        bool Initialize();
        void Shutdown();
        void Run();

        Job* CreateJob(JobFunction function);

        Worker* GetRandomWorker();
        Worker* FindThreadWorker(const std::thread::id threadId);

        static ThreadManager* Get() { return instance; }
    };
}