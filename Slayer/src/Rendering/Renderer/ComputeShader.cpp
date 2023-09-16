#include "Rendering/Renderer/ComputeShader.h"

#include "Core/Core.h"
#include "Rendering/Renderer/Shader.h"

#include "glad/glad.h"

namespace Slayer
{
    void ComputeShader::SetUniform(const std::string& name, const int32_t value)
    {
        int32_t location = GetUniformLocation(name.c_str());
        glUniform1i(location, value);
    }

    int32_t ComputeShader::GetUniformLocation(const std::string& name)
    {
        for (uint32_t i = 0; i < m_uniformCache.size(); i++)
        {
            if (m_uniformCache[i] == name)
                return i;
        }

        int32_t location = glGetUniformLocation(m_id, name.c_str());
        SL_ASSERT(location >= 0 && "Uniform does not exist!");
        m_uniformCache[location] = name;
        return location;
    }

    void ComputeShader::Bind() const
    {
        glUseProgram(m_id);
    }

    void ComputeShader::Unbind() const
    {
        glUseProgram(0);
    }

    void ComputeShader::Dispose() const
    {
        glDeleteProgram(m_id);
    }

    void ComputeShader::Dispatch(uint32_t x, uint32_t y, uint32_t z) const
    {
        glDispatchCompute(x, y, z);
    }

    void ComputeShader::MemoryBarrier(const MemoryBarrierBits barrierBits) const
    {
        GLenum barrierBit = 0;
        switch (barrierBits)
        {
        case MemoryBarrierBits::SL_NONE: break;
        case MemoryBarrierBits::SL_IMAGE_ACCESS: barrierBit = GL_SHADER_IMAGE_ACCESS_BARRIER_BIT; break;
        }

        glMemoryBarrier(barrierBit);
    }

    Shared<ComputeShader> ComputeShader::Create(const std::string& source)
    {
        uint32_t csId = Shader::CompileShader(source, GL_COMPUTE_SHADER);

        uint32_t programId = glCreateProgram();
        glAttachShader(programId, csId);
        glLinkProgram(programId);

        GLint linkSuccess;
        glGetProgramiv(programId, GL_LINK_STATUS, &linkSuccess);
        char infoLog[512];
        glGetProgramInfoLog(programId, 512, nullptr, infoLog);
        if (linkSuccess == GL_FALSE)
        {
            Log::Error("Shader linking failed: " + std::string(infoLog));
            SL_ASSERT(false);
        }

        return MakeShared<ComputeShader>(programId);
    }
}
