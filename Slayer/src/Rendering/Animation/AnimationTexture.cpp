#include "Rendering/Animation/AnimationTexture.h"
#include "glad/glad.h"
#include "Rendering/Animation/AnimationChannel.h"

namespace Slayer {

	Shared<AnimationTexture> AnimationTexture::Create()
	{

		return MakeShared<AnimationTexture>();
	}

}