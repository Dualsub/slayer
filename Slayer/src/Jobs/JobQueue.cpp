#include "Jobs/JobQueue.h"

namespace Slayer
{
    JobQueue::JobQueue()
    {
        m_jobs = {};
        m_bottom = 0;
        m_top = 0;
    }

    JobQueue::JobQueue(std::size_t maxJobs)
    {
        m_jobs.resize(maxJobs);
        m_bottom = 0;
        m_top = 0;
    }

    void JobQueue::Push(Job *job)
    {
        int bottom = m_bottom.load(std::memory_order_seq_cst);
        m_jobs[bottom] = job;
        m_bottom.store(bottom + 1, std::memory_order_seq_cst);
    }

    Job *JobQueue::Pop()
    {
        int bottom = m_bottom.load(std::memory_order_seq_cst);
        bottom = std::max(0, bottom - 1);
        m_bottom.store(bottom, std::memory_order_seq_cst);
        int top = m_top.load(std::memory_order_seq_cst);

        if (top <= bottom)
        {
            Job *job = m_jobs[bottom];

            if (top != bottom)
            {
                return job;
            }

            int stolenTop = top + 1;
            if (m_top.compare_exchange_strong(stolenTop, top + 1, std::memory_order_seq_cst))
            {
                m_bottom.store(stolenTop, std::memory_order_release);
                return nullptr;
            }
            return job;
        }
        else
        {
            m_bottom.store(top, std::memory_order_seq_cst);
            return nullptr;
        }
    }

    Job *JobQueue::Steal()
    {
        int top = m_top.load(std::memory_order_seq_cst);
        int bottom = m_bottom.load(std::memory_order_seq_cst);

        if (m_top < m_bottom)
        {
            Job *job = m_jobs[top];

            if (m_top.compare_exchange_strong(top, top + 1, std::memory_order_seq_cst))
            {
                return job;
            }

            return nullptr;
        }
        else
            return nullptr;
    }

}
