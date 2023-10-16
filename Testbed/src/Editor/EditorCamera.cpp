#include "Editor/EditorCamera.h"
#include "glm/gtc/matrix_transform.hpp"
#include "Core/Core.h"
#include "Input/Input.h"
#include "Input/Keys.h"
#include "Core/Application.h"

namespace Slayer::Editor {

    EditorCamera::EditorCamera()
    {
        position = glm::vec3(0.0f, 0.0f, -500.0f);
        m_forward = glm::vec3(0.0f, 0.0f, -1.0f);
        m_right = glm::cross(m_forward, m_up);
        m_fov = 72.0f;
        IsProjectionDirty = true;
    }

    void EditorCamera::Update(float dt)
    {
        IsProjectionDirty = false;
        if (Slayer::Input::IsKeyPressed(Slayer::SlayerKey::KEY_X))
        {
            IsProjectionDirty = true;
            m_fov += 10.0f * dt;
        }
        if (Slayer::Input::IsKeyPressed(Slayer::SlayerKey::KEY_Z))
        {
            IsProjectionDirty = true;
            m_fov -= 10.0f * dt;
        }

        if (IsProjectionDirty)
        {
            Slayer::Window& window = Slayer::Application::Get()->GetWindow();
            SetProjectionMatrix(m_fov, float(window.GetWidth()), float(window.GetHeight()), nearPlane, farPlane);
        }

        Slayer::Vec3 dr = Slayer::Vec3(0.0f);

        if (Slayer::Input::IsKeyPressed(Slayer::SlayerKey::KEY_W))
        {
            dr += m_forward;
        }

        if (Slayer::Input::IsKeyPressed(Slayer::SlayerKey::KEY_S))
        {
            dr += -m_forward;
        }

        if (Slayer::Input::IsKeyPressed(Slayer::SlayerKey::KEY_D))
        {
            dr += m_right;
        }

        if (Slayer::Input::IsKeyPressed(Slayer::SlayerKey::KEY_A))
        {
            dr += -m_right;
        }

        if (Slayer::Input::IsKeyPressed(Slayer::SlayerKey::KEY_E))
        {
            dr += m_up;
        }

        if (Slayer::Input::IsKeyPressed(Slayer::SlayerKey::KEY_Q))
        {
            dr += -m_up;
        }

        float velocity = m_movementSpeed;

        if (Slayer::Input::IsKeyPressed(Slayer::SlayerKey::KEY_LEFT_SHIFT))
        {
            velocity = 3 * m_movementSpeed;
        }

        if (Slayer::Input::IsKeyPressed(Slayer::SlayerKey::KEY_LEFT_CONTROL))
        {
            velocity = 10 * m_movementSpeed;
        }

        if (dr != Slayer::Vec3(0.0))
            position += (glm::normalize(dr) * velocity * dt);

        if (Slayer::Input::IsKeyPressed(Slayer::SlayerKey::KEY_F1))
            transform = glm::lookAt(position, position + m_forward, m_up);


        transform = glm::lookAt(position, position + m_forward, m_up);
    }

    void EditorCamera::OnEvent(Slayer::Event& e)
    {
        SL_EVENT_DISPATCH(Slayer::KeyPressEvent, EditorCamera::OnKeyPress);
        SL_EVENT_DISPATCH(Slayer::KeyReleaseEvent, EditorCamera::OnKeyRelease);
        SL_EVENT_DISPATCH(Slayer::MouseMoveEvent, EditorCamera::OnMouseMove);
    }

    bool EditorCamera::OnKeyPress(Slayer::KeyPressEvent& e)
    {
        switch (e.key)
        {
        case Slayer::SlayerKey::KEY_F1:
            break;
        case Slayer::SlayerKey::KEY_C:
            m_fov = 90.0f;
            break;
        default:
            break;
        }
        return true;
    }

    bool EditorCamera::OnKeyRelease(Slayer::KeyReleaseEvent& e)
    {
        return true;
    }

    bool EditorCamera::OnMouseMove(Slayer::MouseMoveEvent& e)
    {
        if (m_firstMouse)
        {
            m_lastX = (float)e.posX;
            m_lastY = (float)e.posY;
            m_firstMouse = false;
        }

        float xoffset = (float)e.posX - m_lastX;
        float yoffset = m_lastY - (float)e.posY;
        m_lastX = (float)e.posX;
        m_lastY = (float)e.posY;

        float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        m_yaw += xoffset;
        m_pitch += yoffset;

        if (m_pitch > 89.0f)
            m_pitch = 89.0f;
        if (m_pitch < -89.0f)
            m_pitch = -89.0f;

        glm::vec3 dir;
        dir.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        dir.y = sin(glm::radians(m_pitch));
        dir.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        m_forward = glm::normalize(dir);
        m_right = glm::cross(m_forward, m_up);
        return true;
    }

}