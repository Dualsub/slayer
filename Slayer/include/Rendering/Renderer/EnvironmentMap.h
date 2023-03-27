#pragma once

#include "Core/Core.h"
#include "Rendering/Renderer/Shader.h"
#include "Rendering/Renderer/Texture.h"
#include "Rendering/Renderer/VertexArray.h"

namespace Slayer {

	class EnvironmentMap
	{
		static Shared<VertexArray> cubeVAO;
		static Shared<VertexArray> quadVAO;

		unsigned int envCubemapID;
		unsigned int irradianceMapID;
		unsigned int prefilterMapID;
		unsigned int brdfTextureID;
		Shared<Shader> cubemapShader;
		static unsigned int CreateEnvironmentMap(unsigned int fboID, unsigned int rboID, Shared<Texture> hdrTexture, Shared<Shader> captureShader, const Mat4& captureProjection, const Mat4 captureViews[6]);
		static unsigned int CreateIrradianceMap(unsigned int fboID, unsigned int rboID, unsigned int envCubemapID, Shared<Shader> irradianceShader, const Mat4& captureProjection, const Mat4 captureViews[6]);
		static unsigned int CreatePrefilterMap(unsigned int fboID, unsigned int rboID, unsigned int envCubemapID, Shared<Shader> prefilterShader, const Mat4& captureProjection, const Mat4 captureViews[6]);
		static unsigned int CreateBRDF(unsigned int fboID, unsigned int rboID, Shared<Shader> brdfShader);
	public:
		static Shared<EnvironmentMap> LoadFromFile(const std::string& filePath, Shared<Shader> cubemapShader, Shared<Shader> captureShader, Shared<Shader> irradianceShader, Shared<Shader> prefilterShader, Shared<Shader> brdfShader);
		static Shared<EnvironmentMap> Load(Shared<Texture> hdrTexture, Shared<Shader> cubemapShader, Shared<Shader> captureShader, Shared<Shader> irradianceShader, Shared<Shader> prefilterShader, Shared<Texture> brdfTexture);
		EnvironmentMap(unsigned int envCubemapID, unsigned int irradianceMapID, unsigned int prefilterMapID, unsigned int brdfTextureID, Shared<Shader> cubemapShader);
		void Draw();
		unsigned int GetIrradianceMapID() { return irradianceMapID; }
		unsigned int GetPrefilterMapID() { return prefilterMapID; }
		unsigned int GetBRDFTextureID() { return brdfTextureID; }
	};

}
