#pragma once

#include "Slayer.h"

namespace Slayer {

	class Socket 
	{
	private:
		Mat4 worldTransform = Mat4(1.0f);
	public:
		std::string name = "";
		std::string bone = "";
		Mat4 offset = Mat4(1.0f);

		Socket() = default;
		Socket(const std::string name, const std::string& bone, const Mat4& offset) 
			: name(name), bone(bone), offset(offset) 
		{
		}
		~Socket() = default;

		const Mat4& GetWorldTransform() const { return worldTransform; }
		void SetWorldTransform(const Mat4& inWorldTransform) { worldTransform = inWorldTransform; }

		template<typename Serializer>
		void Transfer(Serializer& serializer)
		{
			SL_TRANSFER_VAR(name);
			SL_TRANSFER_VAR(bone);
			SL_TRANSFER_VAR(offset);
		}
	};

}