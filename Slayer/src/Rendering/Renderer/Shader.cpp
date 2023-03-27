#include "Core/Core.h"
#include "Core/Log.h"
#include "Rendering/Renderer/Shader.h"
#include "glad/glad.h"
#include <sstream>
#include <fstream>


namespace Slayer
{

    Shader::Shader(unsigned int programID, unsigned int vertexShaderID, unsigned int fragmentShaderID):
        programID(programID), vertexShaderID(vertexShaderID), fragmentShaderID(fragmentShaderID)
    {
    }

    Shader::~Shader()
    {
        glDetachShader(programID, vertexShaderID);
        glDeleteShader(vertexShaderID);

        glDetachShader(programID, fragmentShaderID);
        glDeleteShader(fragmentShaderID);

        glDeleteProgram(programID);
    }

    int Shader::CompileShader(const std::string& source, unsigned int type)
    {
        int shaderID = glCreateShader(type);
        const char* sourceShared = source.c_str();
        glShaderSource(shaderID, 1, &sourceShared, nullptr);
        glCompileShader(shaderID);

        int success;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
        char infoLog[512];
        glGetShaderInfoLog(shaderID, 512, nullptr, infoLog);
        if (!success)
        {
                Log::Error("Shader compilation failed: " + std::string(infoLog));
                std::cout << "Source:\n" << source << std::endl;
                SL_ASSERT(false);
        }
#if LOG_VERBOSE
        else {
            std::cout << "SHADER::COMPLIATION_SUCCESS" << std::endl;
            std::cout << infoLog << std::endl;
        }
#endif
        return shaderID;
    }

    void Shader::Bind()
    {
        glUseProgram(programID);
    }

    void Shader::Unbind()
    {
        glUseProgram(0);
    }

    Shared<Shader> Shader::LoadShaderFromFiles(const std::string& vsFile, const std::string& fsFile)
    {
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        // ensure ifstream objects can throw exceptions:
        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            // open files
            vShaderFile.open(vsFile);
            fShaderFile.open(fsFile);
            std::stringstream vShaderStream, fShaderStream;
            // read file's buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            // close file handlers
            vShaderFile.close();
            fShaderFile.close();
            // convert stream into string
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (std::ifstream::failure e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        }
#if LOG_VERBOSE
        std::cout << "Loading: " << vsFile << std::endl;
        std::cout << "Loading: " << fsFile << std::endl;

        auto shader = LoadShader(vertexCode, fragmentCode);
        shader->vsName = vsFile + vertexCode;
        shader->fsName = fsFile + fragmentCode;
        return shader;
#else
        return LoadShader(vertexCode, fragmentCode);
#endif
    }

    void Shader::SetUniform(const std::string& name, const bool value)
    {
    }

    void Shader::SetUniform(const std::string& name, const int value)
    {
        int loc = glGetUniformLocation(programID, name.c_str());
#if ASSERT_UNIFORM
        assert(loc != -1);
#endif
        glUniform1i(loc, value);
    }

    void Shader::SetUniform(const std::string& name, const float value)
    {
        int loc = glGetUniformLocation(programID, name.c_str());
#if ASSERT_UNIFORM
        assert(loc != -1);
#endif
        glUniform1f(loc, value);
    }

    void Shader::SetUniform(const std::string& name, const Vec2& value)
    {
        int loc = glGetUniformLocation(programID, name.c_str());
#if ASSERT_UNIFORM
        assert(loc != -1);
#endif
        glUniform2f(loc, value.x, value.y);
    }

    void Shader::SetUniform(const std::string& name, const Vec3& value)
    {
        int loc = glGetUniformLocation(programID, name.c_str());
#if ASSERT_UNIFORM
        assert(loc != -1);
#endif
        glUniform3f(loc, value.x, value.y, value.z);
    }

    void Shader::SetUniform(const std::string& name, const Vec4& value)
    {
        int loc = glGetUniformLocation(programID, name.c_str());
#if ASSERT_UNIFORM
        assert(loc != -1);
#endif
        glUniform4f(loc, value.x, value.y, value.z, value.w);
    }

    void Shader::SetUniform(const std::string& name, const Mat3& value)
    {
        int loc = glGetUniformLocation(programID, name.c_str());
#if ASSERT_UNIFORM
        assert(loc != -1);
#endif
        glUniformMatrix3fv(loc, 1, GL_FALSE, &value[0][0]);
    }

    void Shader::SetUniform(const std::string& name, const Mat4& value)
    {
        int loc = glGetUniformLocation(programID, name.c_str());
#if ASSERT_UNIFORM
        assert(loc != -1);
#endif
        glUniformMatrix4fv(loc, 1, GL_FALSE, &value[0][0]);
    }

    void Shader::Dispose()
    {
    }

    Shared<Shader> Shader::LoadShader(const std::string& vs, const std::string& fs)
    {
        int programID = glCreateProgram();
        int vsID = CompileShader(vs, GL_VERTEX_SHADER);
        int fsID = CompileShader(fs, GL_FRAGMENT_SHADER);

        glAttachShader(programID, vsID);
        glAttachShader(programID, fsID);
        glLinkProgram(programID);

        return std::make_shared<Shader>(programID, vsID, fsID);
    }

}

