#include "Jobs/JobManager.h"

namespace Slayer
{
    JobManager *JobManager::instance = nullptr;

    bool JobManager::Initialize()
    {
        return true;
    }

    void JobManager::Shutdown()
    {
    }
}