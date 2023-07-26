#pragma once

#include "Core/Core.h"
#include "Resources/Asset.h"

#define LOG_VERBOSE 1
#define ASSERT_UNIFORM 0

namespace Slayer
{
	class Shader
	{
	private:
		Dict<std::string, int> uniformLocations = {};
		int programID = 0;
		int vertexShaderID = 0;
		int fragmentShaderID = 0;
#if LOG_VERBOSE
		std::string vsSource;
		std::string fsSource;
#endif
		static int CompileShader(const std::string& source, unsigned int type);
		int GetUniformLocation(const std::string& name);
	public:
		AssetID assetID;

		Shader(int programID, int vertexShaderID, int fragmentShaderID);
		~Shader();

		void Bind();
		void BindWithCheck();
		void Unbind();

		bool IsBound();
		bool HasUniform(const std::string& name);
		inline unsigned int GetID() { return programID; };

		void SetUniform(const std::string& name, const bool value);
		void SetUniform(const std::string& name, const int value);
		void SetUniform(const std::string& name, const int* values, size_t count);
		void SetUniform(const std::string& name, const float value);
		void SetUniform(const std::string& name, const float* values, size_t count);
		void SetUniform(const std::string& name, const Vec2& value);
		void SetUniform(const std::string& name, const Vec2i& value);
		void SetUniform(const std::string& name, const Vec2i* values, size_t count);
		void SetUniform(const std::string& name, const Vec3& value);
		void SetUniform(const std::string& name, const Vec4& value);
		void SetUniform(const std::string& name, const Mat3& value);
		void SetUniform(const std::string& name, const Mat4& value);
		void SetUniform(const std::string& name, const Mat4* values, size_t count);

		void Dispose();

		static Shared<Shader> LoadShaderFromFiles(const std::string& vsFile, const std::string& fsFile);
		static Shared<Shader> LoadShader(const std::string& vs, const std::string& fs);
	};

}

