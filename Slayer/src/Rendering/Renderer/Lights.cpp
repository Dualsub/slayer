#include "Rendering/Renderer/Lights.h"

namespace Slayer {

	PointLight::PointLight(Vec3 position, Vec3 color)
		: position(position, 0.0f), color(color, 0.0f)
	{
	}

	DirectionalLight::DirectionalLight(Vec3 direction, Vec3 color)
		: direction(direction, 0.0f), color(color, 0.0f)
	{
	}
	SpotLight::SpotLight(Vec3 position, float angle, Vec3 color)
		: positionAndAngle(position, angle), color(color, 1.0f)
	{
	}

}