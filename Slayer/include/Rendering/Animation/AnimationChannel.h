#pragma once 

#include "Slayer.h"

namespace Slayer {
#pragma pack(push, 1)
	template<typename T>
	struct Frame
	{
		float time = 0.0f;
		T value;
	};
#pragma pack(pop)

	class AnimationChannel
	{
	private:
		std::string name;
		Vector<Frame<Vec3>> positions;
		Vector<Frame<Quat>> rotations;
		Vector<Frame<Vec3>> scales;

		template<typename T>
		T Sample(const Vector<Frame<T>>& frames, float time) const
		{
			if (frames.size() == 1)
				return frames[0].value;
			for (size_t i = 0; i < frames.size() - 1; i++)
			{
				if (time >= frames[i].time && time <= frames[i + 1].time)
				{
					float t = (time - frames[i].time) / (frames[i + 1].time - frames[i].time);
					return glm::mix(frames[i].value, frames[i + 1].value, t);
				}
			}

			return frames[frames.size() - 1].value;
		}

		template<>
		Quat Sample(const Vector<Frame<Quat>>& frames, float time) const
		{
			if (frames.size() == 1)
				return frames[0].value;
			for (size_t i = 0; i < frames.size() - 1; i++)
			{
				if (time >= frames[i].time && time <= frames[i + 1].time)
				{
					float t = (time - frames[i].time) / (frames[i + 1].time - frames[i].time);
					return glm::slerp(frames[i].value, frames[i + 1].value, t);
				}
			}

			return frames[frames.size() - 1].value;
		}

	public:
		AnimationChannel(const std::string& name, const Vector<Frame<Vec3>>& positions, const Vector<Frame<Quat>>& rotations, const Vector<Frame<Vec3>>& scales)
			: name(name), positions(positions), rotations(rotations), scales(scales)
		{
		}
		AnimationChannel() = default;
		virtual ~AnimationChannel() = default;

		Mat4 Sample(float time) const
		{
			Mat4 transform = Mat4(1.0f);
			transform = glm::translate(transform, Sample(positions, time));
			transform *= glm::toMat4(glm::normalize(Sample(rotations, time)));
			return transform;
		}
	};
}