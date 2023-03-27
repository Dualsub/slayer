#pragma once

#include <atomic>
#include <array>

namespace Slayer
{
    using JobFunction = void (*)(class Job &);

    class Job
    {
    private:
        JobFunction function;
        Job *parent;
        std::atomic_int unfinishedJobs;
        static constexpr std::size_t JOB_PAYLOAD_SIZE = sizeof(function) + sizeof(parent) + sizeof(unfinishedJobs);
        static constexpr std::size_t JOB_MAX_PADDING_SIZE = 64;
        static constexpr std::size_t JOB_PADDING_SIZE = JOB_MAX_PADDING_SIZE - JOB_PAYLOAD_SIZE;

        std::array<unsigned char, JOB_PADDING_SIZE> padding;

    public:
        Job(JobFunction function, Job *parent);
        Job(const Job &) = delete;
        Job() = default;
        ~Job() = default;

        template<typename T>
        void SetData(const T &data)
        {
            static_assert(sizeof(T) <= JOB_PADDING_SIZE, "Job data is too large");
            memcpy(padding.data(), &data, sizeof(T));
        }

        template<typename T>
        T GetDataCopy() const
        {
            static_assert(sizeof(T) <= JOB_PADDING_SIZE, "Job data is too large");
            T data;
            memcpy(&data, padding.data(), sizeof(T));
            return data;
        }

        template<typename T>
        T* GetData() const
        {
            static_assert(sizeof(T) <= JOB_PADDING_SIZE, "Job data is too large");
            return reinterpret_cast<T*>(padding.data());
        }

        void Run();
        bool Finished() const;
        void Finish();
    };
}