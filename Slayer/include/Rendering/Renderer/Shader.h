#pragma once

#include "Core/Core.h"
#include "Resources/Asset.h"

#define LOG_VERBOSE 0
#define ASSERT_UNIFORM 0

namespace Slayer
{
	class Shader
	{
	private:
		unsigned int programID = 0;
		unsigned int vertexShaderID = 0;
		unsigned int fragmentShaderID = 0;
#if LOG_VERBOSE
		std::string vsName;
		std::string fsName;
#endif
		static int CompileShader(const std::string& source, unsigned int type);

	public:
		AssetID assetID;

		Shader(unsigned int programID, unsigned int vertexShaderID, unsigned int fragmentShaderID);
		~Shader();

		void Bind();
		void Unbind();

		inline unsigned int GetID() { return programID; };

		void SetUniform(const std::string& name, const bool value);
		void SetUniform(const std::string& name, const int value);
		void SetUniform(const std::string& name, const float value);
		void SetUniform(const std::string& name, const Vec2& value);
		void SetUniform(const std::string& name, const Vec3& value);
		void SetUniform(const std::string& name, const Vec4& value);
		void SetUniform(const std::string& name, const Mat3& value);
		void SetUniform(const std::string& name, const Mat4& value);

		void Dispose();

		static Shared<Shader> LoadShaderFromFiles(const std::string& vsFile, const std::string& fsFile);
		static Shared<Shader> LoadShader(const std::string& vs, const std::string& fs);

	};

}

