#pragma once

#include "Core/Core.h"

// We wrap the STL containers in our own namespace to avoid conflicts with other libraries 
// and to make it easier to change the underlying implementation if needed.

namespace Slayer
{
    template<typename T>
    using Vector = std::vector<T>;
    template<typename T, size_t N>
    using Array = std::array<T, N>;
    template<typename T, typename U>
    using Dict = std::unordered_map<T, U>;
    template<typename T, typename U, typename Hash>
    using DictHash = std::unordered_map<T, U, Hash>;
    template<typename T, typename U>
    using Map = std::map<T, U>;
    template<typename T>
    using Set = std::set<T>;
    template<typename T, typename U>
    using Tuple = std::tuple<T, U>;



    template<typename T, size_t N>
    class FixedVector
    {
        T m_data[N];
        size_t m_size = 0;
    public:
        FixedVector() = default;
        FixedVector(const FixedVector& other) = default;
        FixedVector(FixedVector&& other) noexcept = default;
        FixedVector& operator=(const FixedVector& other) = default;
        FixedVector& operator=(FixedVector&& other) noexcept = default;
        ~FixedVector() = default;

        void PushBack(const T& value)
        {
            SL_ASSERT(m_size < N && "FixedVector is full!");
            m_data[m_size++] = value;
        }

        void PushBack(T&& value)
        {
            SL_ASSERT(m_size < N && "FixedVector is full!");
            m_data[m_size++] = std::move(value);
        }

        void PopBack()
        {
            SL_ASSERT(m_size > 0 && "FixedVector is empty!");
            m_size--;
        }

        void Clear()
        {
            m_size = 0;
        }

        T& operator[](size_t index)
        {
            SL_ASSERT(index < m_size && "Index out of bounds!");
            return m_data[index];
        }

        const T& operator[](size_t index) const
        {
            SL_ASSERT(index < m_size && "Index out of bounds!");
            return m_data[index];
        }

        T& Front()
        {
            SL_ASSERT(m_size > 0 && "FixedVector is empty!");
            return m_data[0];
        }

        const T& Front() const
        {
            SL_ASSERT(m_size > 0 && "FixedVector is empty!");
            return m_data[0];
        }

        T& Back()
        {
            SL_ASSERT(m_size > 0 && "FixedVector is empty!");
            return m_data[m_size - 1];
        }

        const T& Back() const
        {
            SL_ASSERT(m_size > 0 && "FixedVector is empty!");
            return m_data[m_size - 1];
        }

        size_t Size() const
        {
            return m_size;
        }

        size_t Capacity() const
        {
            return N;
        }

        bool Empty() const
        {
            return m_size == 0;
        }

        T* Data()
        {
            return m_data;
        }

        const T* Data() const
        {
            return m_data;
        }

        size_t FindIndexOf(const T& value)
        {
            for (size_t i = 0; i < m_size; i++)
            {
                if (m_data[i] == value)
                    return i;
            }
            return -1;
        }

    };
}
