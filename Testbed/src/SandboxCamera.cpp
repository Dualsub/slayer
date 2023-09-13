#include "SandboxCamera.h"
#include "glm/gtc/matrix_transform.hpp"
#include "Core/Core.h"
#include "Input/Input.h"
#include "Input/Keys.h"
#include "Core/Application.h"
#include "Testbed.h"

namespace Testbed {

	SandboxCamera::SandboxCamera(float movementSpeed)
		: movementSpeed(movementSpeed)
	{
		position = glm::vec3(0.0f, 0.0f, -500.0f);
		forward = glm::vec3(0.0f, 0.0f, -1.0f);
		right = glm::cross(forward, up);
		velocity = glm::vec3(0.0f);
	}

	void SandboxCamera::Update(float dt)
	{
		IsProjectionDirty = false;
		if (Slayer::Input::IsKeyPressed(Slayer::SlayerKey::KEY_X))
		{
			IsProjectionDirty = true;
			fov += 10.0f * dt;
		}
		if (Slayer::Input::IsKeyPressed(Slayer::SlayerKey::KEY_Z))
		{
			IsProjectionDirty = true;
			fov -= 10.0f * dt;
		}

		if (IsProjectionDirty)
		{
			Slayer::Window& window = Slayer::Application::Get<Testbed::TestbedApplication>()->GetWindow();
			Slayer::Log::Info("FOV:", fov);
			SetProjectionMatrix(fov, float(window.GetWidth()), float(window.GetHeight()), nearPlane, farPlane);
		}

		Slayer::Vec3 dr = Slayer::Vec3(0.0f);

		if (Slayer::Input::IsKeyPressed(Slayer::SlayerKey::KEY_W))
		{
			dr += forward;
		}

		if (Slayer::Input::IsKeyPressed(Slayer::SlayerKey::KEY_S))
		{
			dr += -forward;
		}

		if (Slayer::Input::IsKeyPressed(Slayer::SlayerKey::KEY_D))
		{
			dr += right;
		}

		if (Slayer::Input::IsKeyPressed(Slayer::SlayerKey::KEY_A))
		{
			dr += -right;
		}

		if (Slayer::Input::IsKeyPressed(Slayer::SlayerKey::KEY_E))
		{
			dr += up;
		}

		if (Slayer::Input::IsKeyPressed(Slayer::SlayerKey::KEY_Q))
		{
			dr += -up;
		}

		float velocity = movementSpeed;

		if (Slayer::Input::IsKeyPressed(Slayer::SlayerKey::KEY_LEFT_SHIFT))
		{
			velocity = 3 * movementSpeed;
		}

		if (Slayer::Input::IsKeyPressed(Slayer::SlayerKey::KEY_LEFT_CONTROL))
		{
			velocity = 10 * movementSpeed;
		}

		if (dr != Slayer::Vec3(0.0))
			position += (glm::normalize(dr) * velocity * dt);

		if (Slayer::Input::IsKeyPressed(Slayer::SlayerKey::KEY_F1))
			transform = glm::lookAt(position, position + forward, up);


		transform = glm::lookAt(position, position + forward, up);
	}

	void SandboxCamera::OnEvent(Slayer::Event& e)
	{
		SL_EVENT_DISPATCH(Slayer::KeyPressEvent, SandboxCamera::OnKeyPress);
		SL_EVENT_DISPATCH(Slayer::KeyReleaseEvent, SandboxCamera::OnKeyRelease);
		SL_EVENT_DISPATCH(Slayer::MouseMoveEvent, SandboxCamera::OnMouseMove);
	}

	bool SandboxCamera::OnKeyPress(Slayer::KeyPressEvent& e)
	{
		switch (e.key)
		{
		case Slayer::SlayerKey::KEY_F1:
			break;
		case Slayer::SlayerKey::KEY_TAB:
			//Engine::Get()->GetWindow()->SetCursorMode(editMode ? CursorMode::SL_DISABLED : CursorMode::SL_NORMAL);
			//editMode = !editMode;
			//firstMouse = true;
			break;
		case Slayer::SlayerKey::KEY_C:
			fov = 90.0f;
			break;
		default:
			break;
		}
		return true;
	}

	bool SandboxCamera::OnKeyRelease(Slayer::KeyReleaseEvent& e)
	{
		/*switch (e.key)
		{
		case 83:
		case 87:
		case 65:
		case 68:
		case 81:
		case 69:
			velocity = Vec3(0.0f);
			break;
		}*/
		return true;
	}

	bool SandboxCamera::OnMouseMove(Slayer::MouseMoveEvent& e)
	{
		if (firstMouse)
		{
			lastX = (float)e.posX;
			lastY = (float)e.posY;
			firstMouse = false;
		}

		float xoffset = (float)e.posX - lastX;
		float yoffset = lastY - (float)e.posY;
		lastX = (float)e.posX;
		lastY = (float)e.posY;

		float sensitivity = 0.1f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw += xoffset;
		pitch += yoffset;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		glm::vec3 dir;
		dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		dir.y = sin(glm::radians(pitch));
		dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		forward = glm::normalize(dir);
		right = glm::cross(forward, up);
		return true;
	}

}