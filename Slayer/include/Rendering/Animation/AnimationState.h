#pragma once

#include "Slayer.h"

#define SL_MAX_BLEND_ANIMATIONS 2

namespace Slayer {

	struct AnimationState
	{
		AnimationState() = default;
		~AnimationState() = default;

		float weights[SL_MAX_BLEND_ANIMATIONS] = { 0.0f };
		int32_t textureIDs[SL_MAX_BLEND_ANIMATIONS] = { -1 };
		float times[SL_MAX_BLEND_ANIMATIONS] = { 0.0f };
		Vec2i frames[SL_MAX_BLEND_ANIMATIONS] = { {0, 0} };

		Mat4* inverseBindPose;
		int32_t* parents;

		void SetAnimation(uint32_t index, int32_t textureID, float time, const Vec2i& frames, float weight = 1.0f)
		{
			this->textureIDs[index] = textureID;
			this->times[index] = time;
			this->frames[index] = frames;
			this->weights[index] = weight;
		}
	};
}