#pragma once

#include "Slayer.h"
#include "AnimationChannel.h"

namespace Slayer {

	class Animation
	{
	private:
		float duration = 0.0f;
		
		// GPU Animation Data
		uint32_t textureID = 0;
		uint32_t numChannels = 0;
		Vector<float> times = {};
	public:
		Animation(uint32_t textureID, uint32_t numChannels, float duration, const Vector<float>& times)
			: textureID(textureID), numChannels(numChannels), duration(duration), times(times)
		{
		}
		~Animation() = default;
		
		static Shared<Animation> Create(const Vector<float>& data, const Vector<float>& times, float duration);
		
		float GetDuration() const { return duration; }
		const Vector<float>& GetTimes() const { return times; }
		uint32_t GetTextureID() const { return textureID; }
	};
}