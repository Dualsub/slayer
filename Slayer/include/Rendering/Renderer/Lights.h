#pragma once

#include "Core/Core.h"
#include "Core/Containers.h"
#include "Core/Math.h"
#include "Resources/Asset.h"
#include "Serialization/Serialization.h"

#define MAX_POINT_LIGHTS 64
#define SL_SHADOW_CASCADES 4

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
		constexpr static uint32_t NUM_FLOAT_PAD = 4;
		DirectionalLightData directionalLight;
		PointLightData pointLights[MAX_POINT_LIGHTS];
		Mat4 lightSpaceMatrices[SL_SHADOW_CASCADES];
		float shadowCascadeEnds[SL_SHADOW_CASCADES * NUM_FLOAT_PAD] = { 0 }; // 4 floats for padding
		int32_t numLights = 0;

		LightsData() = default;
		~LightsData() = default;

		LightsData(const DirectionalLightData& directionalLight, const Vector<PointLightData>& pointLights, const Array<Mat4, SL_SHADOW_CASCADES>& lightSpaceMatrices, const Array<float, SL_SHADOW_CASCADES>& shadowCascadeEnds)
			: directionalLight(directionalLight), numLights(uint32_t(pointLights.size()))
		{
			Copy(pointLights.data(), this->pointLights, pointLights.size() * sizeof(PointLightData));
			Copy(lightSpaceMatrices.data(), this->lightSpaceMatrices, SL_SHADOW_CASCADES * sizeof(Mat4));
			for (int i = 0; i < SL_SHADOW_CASCADES; i++)
			{
				this->shadowCascadeEnds[i * NUM_FLOAT_PAD] = shadowCascadeEnds[i];
			}
		}
	};

}