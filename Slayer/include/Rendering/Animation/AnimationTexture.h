#pragma once

#include "Core/Core.h"
#include "Core/Containers.h"
#include "Core/Math.h"
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