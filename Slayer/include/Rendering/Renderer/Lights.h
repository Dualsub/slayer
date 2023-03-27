#pragma once

#include "Core/Core.h"
#include "Serialization/Serialization.h"
#include "Core/Math.h"
#include "Resources/Asset.h"

#define MAX_POINT_LIGHTS 64

namespace Slayer {

	struct PointLight
	{
		PointLight() = default;
		PointLight(Vec3 position, Vec3 color);
		Vec4 position;
		Vec4 color;
	};

	struct DirectionalLight
	{
		DirectionalLight() = default;
		DirectionalLight(Vec3 direction, Vec3 color);
		Vec4 direction;
		Vec4 color;
		template<typename Serializer>
		void Transfer(Serializer& serializer)
		{
			SL_TRANSFER_VAR(direction);
			SL_TRANSFER_VAR(color);
		}
	};

	struct SpotLight
	{
		SpotLight(Vec3 position, float angle, Vec3 color);
		Vec4 positionAndAngle;
		Vec4 color;
	};

	struct LightsData
	{
		DirectionalLight directionalLight;
		PointLight pointLights[MAX_POINT_LIGHTS];
		Mat4 lightSpaceMatrix;
		int numLights;
		LightsData(const DirectionalLight& directionalLight, const Vector<PointLight>& inPointLights, const Mat4& lightSpaceMatrix) 
			: directionalLight(directionalLight), lightSpaceMatrix(lightSpaceMatrix)
		{
			std::copy(inPointLights.begin(), inPointLights.end(), pointLights);
			numLights = (int)inPointLights.size();
		}
	};

}