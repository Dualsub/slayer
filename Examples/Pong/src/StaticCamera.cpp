#include "StaticCamera.h"
#include "glm/gtc/matrix_transform.hpp"
#include "Core/Core.h"
#include "Input/Input.h"
#include "Input/Keys.h"
#include "Core/Application.h"
#include "Pong.h"

namespace Pong {

    StaticCamera::StaticCamera()
    {
        position = glm::vec3(0.0f, 0.0f, -500.0f);
        forward = glm::vec3(0.0f, 0.0f, 1.0f);
        right = glm::cross(forward, up);
        transform = glm::lookAt(position, position + forward, up);
        Slayer::Window& window = Slayer::Application::Get<Pong::PongApplication>()->GetWindow();
        SetProjectionMatrix(fov, float(window.GetWidth()), float(window.GetHeight()), nearPlane, farPlane);
    }
}