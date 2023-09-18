#pragma once

#include "Core/Core.h"
#include "Jobs/Job.h"
#include "Jobs/JobQueue.h"
#include "Jobs/JobPool.h"
#include <thread>
#include <atomic>

namespace Slayer
{
    class Worker
    {
    public:
        enum class Mode
        {
            BACKGROUND,
            FOREGROUND
        };

        enum class State
        {
            IDLE,
            RUNNING,
            STOPPING
        };

    private:
        Unique<JobQueue> workQueue;
        JobPool jobPool;
        std::thread::id threadId;
        std::thread* thread;
        std::atomic<State> state;
        std::atomic<Mode> mode;

        Job* GetJob();

    public:
        Worker(std::size_t maxJobs, Mode mode = Mode::BACKGROUND);
        ~Worker();

        void Run();
        void StartBackgroundWorker();
        void Start();
        void Stop();
        void Submit(Job* job);
        void Wait(Job* job);

        bool IsRunning() const { return state == State::RUNNING; }
        const std::thread::id& Worker::GetThreadId() const { return threadId; }
        JobPool& Pool() { return jobPool; }
    };
}