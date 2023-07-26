#include "Rendering/Animation/Animation.h"
#include "glad/glad.h"
#include <fstream>


namespace Slayer {

	Shared<Animation> Animation::Create(const Vector<float>& data, const Vector<float>& times, float duration)
	{
		uint32_t textureID = 0;

		// Animation texture: frames x (channels * 3 vectors) x 4 floats per vector
		uint32_t numChannels = data.size() / times.size() / 4;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, times.size(), numChannels, 0, GL_RGBA, GL_FLOAT, data.data());

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

		Shared<Animation> animation = MakeShared<Animation>(textureID, numChannels, duration, times);

		return animation;
	}
}
