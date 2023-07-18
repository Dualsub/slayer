#pragma once

#include "Slayer.h"

namespace Slayer {
	struct AnimationState
	{
		AnimationState() = default;
		~AnimationState() = default;

		uint32_t textureID = 0;
		float time = 0.0f;
		Vec2i frames = { 0, 0 };
		Mat4* inverseBindPose;
		int32_t* parents;
	};
}