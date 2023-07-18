#include "Rendering/Animation/Animation.h"
#include "glad/glad.h"
#include <fstream>


namespace Slayer {

	Shared<Animation> Animation::Create(const Vector<float>& data, const Vector<float>& times, float duration)
	{
		uint32_t textureID = 0;

		// Animation texture: frames x (channels * 3 vectors) x 4 floats per vector
		uint32_t numChannels = data.size() / times.size() / 4;

		/*std::fstream file("animation.txt", std::ios::out);
		for (uint32_t i = 0; i < times.size(); i++)
		{
			file << "Time:" << times[i] << std::endl;
			for (uint32_t j = 0; j < numChannels / 3; j++)
			{
				file << "Channel:" << j << std::endl;
				for (uint32_t k = 0; k < 3; k++)
				{
					Vec4 vec;
					Copy(data.data() + (i * numChannels + j * 3 + k) * 4, &vec, sizeof(Vec4));
					file << vec << std::endl;
				}
			}
			file << std::endl;
			file << std::endl;
		}*/


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
