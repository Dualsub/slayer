#pragma once

#include "Rendering/Renderer/Camera.h"
#include "Core/Events.h"
#include "Core/Math.h"

namespace Testbed {

	class SandboxCamera: public Slayer::Camera
	{
	public:
		SandboxCamera(float movementSpeed);
		Slayer::Vec3 velocity;
		Slayer::Vec3 forward;
		Slayer::Vec3 right;
		const Slayer::Vec3 up = Slayer::Vec3(0.0f, 1.0f, 0.0f);
		float yaw = -90.0f;
		float pitch = 0.0f;
		float lastX = 0.0f;
		float lastY = 0.0f;
		float fov = 90.0f;
		bool firstMouse = true;
		bool editMode = false;
		float movementSpeed;
		void OnEvent(Slayer::Event& e);
		bool OnKeyPress(Slayer::KeyPressEvent& e);
		bool OnKeyRelease(Slayer::KeyReleaseEvent& e);
		bool OnMouseMove(Slayer::MouseMoveEvent& e);
		void Update(float dt);
	};

}