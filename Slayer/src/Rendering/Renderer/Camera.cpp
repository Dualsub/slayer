#include "Rendering/Renderer/Camera.h"
#include "glm/gtc/matrix_transform.hpp"

namespace Slayer {

	const Mat4& Camera::GetViewMatrix()
	{
		return transform;
	}

	void Camera::SetViewMatrix(const Mat4& view)
	{
		transform = view;
	}

	const Mat4& Camera::GetProjectionMatrix()
	{
		return projection;
	}

	void Camera::SetProjectionMatrix(float angle, float width, float height, float nearPlane, float farPlane)
	{
		float aspectRatio = width / height;
		projection = glm::perspective(glm::radians(angle), aspectRatio, nearPlane, farPlane);

		IsProjectionDirty = true;

		this->nearPlane = nearPlane;
		this->farPlane = farPlane;
		this->height = height;
		this->width = width;
		this->fov = angle;
	}

	void Camera::SetProjectionMatrix()
	{
		SL_ASSERT(fov && width && height && nearPlane && farPlane && "Camera projection matrix not set!");
		SetProjectionMatrix(fov, width, height, nearPlane, farPlane);
	}
}