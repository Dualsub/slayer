#pragma once

#include "Slayer.h"
#include "AnimationChannel.h"

namespace Slayer {

	class Animation
	{
	private:
		float duration = 0.0f;
		float ticksPerSecond = 0.0f;
		Dict<std::string, AnimationChannel> channels;
	public:
		Animation(float duration, float ticksPerSecond, const Dict<std::string, AnimationChannel>& channels)
			: duration(duration), ticksPerSecond(ticksPerSecond), channels(channels)
		{
		}
		~Animation() = default;
		const Dict<std::string, AnimationChannel>& GetChannels() { return channels; }
		
		const AnimationChannel& GetChannel(const std::string& name)
		{
			SL_ASSERT(channels.find(name) != channels.end() && "Could not find channel.");
			return channels[name];
		}

		Mat4 SampleChannel(const std::string& name, float time) const
		{
			SL_ASSERT(channels.find(name) != channels.end() && "Could not find channel.");
			return channels.at(name).Sample(time);
		}

		float GetDuration() const { return duration; }
		float GetTicksPerSecond() const { return ticksPerSecond; }
	};
}