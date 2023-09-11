#pragma once

#include "Core/Core.h"

namespace Slayer
{
    class ComputeShader
    {
    private:
        uint32_t m_id = 0;

    public:
        ComputeShader(const int32_t id) : m_id(id) {};
        ComputeShader() = default;
        ~ComputeShader() = default;

        static Shared<ComputeShader> Create(const std::string& source);
    };
}
