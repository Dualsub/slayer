#pragma once

#include "Slayer.h"
#include "Rendering/Animation/AnimationChannel.h"

namespace Slayer {

	class AnimationTexture
	{
	private:
		int tboID = 0;
	public:

		AnimationTexture() = default;
		~AnimationTexture() = default;

		static Shared<AnimationTexture> Create();
	};

}