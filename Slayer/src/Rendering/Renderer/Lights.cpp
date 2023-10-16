#include "Rendering/Renderer/Lights.h"

namespace Slayer {

	PointLightData::PointLightData(Vec3 position, Vec3 color)
		: position(position, 0.0f), color(color, 0.0f)
	{
	}

	DirectionalLightData::DirectionalLightData(Vec3 direction, Vec3 color)
		: direction(direction, 0.0f), color(color, 0.0f)
	{
	}
	SpotLightData::SpotLightData(Vec3 position, float angle, Vec3 color)
		: positionAndAngle(position, angle), color(color, 1.0f)
	{
	}

}