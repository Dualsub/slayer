#pragma once

#include "Core/Core.h"
#include "Core/Containers.h"

namespace Slayer
{
    enum class MemoryBarrierBits : uint32_t
    {
        SL_NONE = 0,
        SL_IMAGE_ACCESS,
    };

    class ComputeShader
    {
    private:
        uint32_t m_id = 0;
        std::array<std::string, 20> m_uniformCache = {};
    public:
        ComputeShader(const int32_t id) : m_id(id) {};
        ComputeShader() = default;
        ~ComputeShader() = default;

        void SetUniform(const std::string& name, const int32_t value);
        int32_t GetUniformLocation(const std::string& name);

        void Bind() const;
        void Unbind() const;
        void Dispose() const;

        void Dispatch(uint32_t x, uint32_t y, uint32_t z) const;
        void MemoryBarrier(const MemoryBarrierBits barrierBits) const;

        static Shared<ComputeShader> Create(const std::string& source);
    };
}
