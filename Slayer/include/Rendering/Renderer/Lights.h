#pragma once

#include "Core/Core.h"
#include "Core/Containers.h"
#include "Core/Math.h"
#include "Resources/Asset.h"
#include "Serialization/Serialization.h"

#define MAX_POINT_LIGHTS 64

namespace Slayer {

	struct PointLightData
	{
		PointLightData() = default;
		PointLightData(Vec3 position, Vec3 color);
		Vec4 position;
		Vec4 color;
	};

	struct DirectionalLightData
	{
		DirectionalLightData() = default;
		DirectionalLightData(Vec3 direction, Vec3 color);
		Vec4 direction;
		Vec4 color;
	};

	struct SpotLightData
	{
		SpotLightData(Vec3 position, float angle, Vec3 color);
		Vec4 positionAndAngle;
		Vec4 color;
	};

	struct LightsData
	{
		DirectionalLightData directionalLight;
		PointLightData pointLights[MAX_POINT_LIGHTS];
		Mat4 lightSpaceMatrix;
		int numLights;
		LightsData(const DirectionalLightData& directionalLight, const Vector<PointLightData>& inPointLights, const Mat4& lightSpaceMatrix)
			: directionalLight(directionalLight), lightSpaceMatrix(lightSpaceMatrix)
		{
			std::copy(inPointLights.begin(), inPointLights.end(), pointLights);
			numLights = (int)inPointLights.size();
		}
	};

}