#pragma once

#include "Core/Core.h"
#include "Core/Math.h"

namespace Slayer {

	class Camera
	{
	protected:
		float width;
		float height;
		float nearPlane;
		float farPlane;
		float fov;
		Slayer::Mat4 transform;
		Slayer::Mat4 projection;
		Slayer::Vec3 position;
		Slayer::Quat rotation;
		bool IsProjectionDirty = true;
	public:
		const float GetFov() { return fov; }
		void SetFov(float fov) 
		{ 
			this->fov = fov;
			SetProjectionMatrix();
		}
		const Slayer::Mat4& GetViewMatrix();
		void SetViewMatrix(const Slayer::Mat4& view);
		inline const Vec3& GetPosition() { return position; }
		inline const Quat& GetRotation() { return rotation; }
		inline const float GetRotationEulerY() 
		{
			Slayer::Vec3 euler;
			euler = glm::eulerAngles(rotation) * (180.0f / glm::pi<float>());
			return euler.y;
		}
		void SetPosition(const Vec3& position) { this->position = position; }
		void SetRotation(const Quat& rotation) { this->rotation = rotation; }
		const Slayer::Mat4& GetProjectionMatrix();
		void SetProjectionMatrix(float angle, float width, float height, float nearPlane, float farPlane);
		void SetProjectionMatrix();
	};
}
