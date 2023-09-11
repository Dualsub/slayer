#include "Rendering/Renderer/ComputeShader.h"

#include "Core/Core.h"
#include "Rendering/Renderer/Shader.h"

#include "glad/glad.h"

namespace Slayer
{
    Shared<ComputeShader> ComputeShader::Create(const std::string& source)
    {
        uint32_t csId = Shader::CompileShader(source, GL_COMPUTE_SHADER);

        uint32_t programId = glCreateProgram();
        glAttachShader(programId, csId);
        glLinkProgram(programId);

        return MakeShared<ComputeShader>(programId);
    }
}
