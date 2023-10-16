#pragma once

#include "Rendering/Renderer/Camera.h"
#include "Core/Events.h"
#include "Core/Math.h"

namespace Slayer::Editor {

    class EditorCamera : public Slayer::Camera
    {
    private:
        Slayer::Vec3 m_forward;
        Slayer::Vec3 m_right;
        const Slayer::Vec3 m_up = Slayer::Vec3(0.0f, 1.0f, 0.0f);
        float m_yaw = -90.0f;
        float m_pitch = 0.0f;
        float m_lastX = 0.0f;
        float m_lastY = 0.0f;
        float m_fov = 72.0f;
        bool m_firstMouse = true;
        float m_movementSpeed = 100.0f;
    public:
        EditorCamera();
        ~EditorCamera() = default;

        void OnEvent(Slayer::Event& e);
        bool OnKeyPress(Slayer::KeyPressEvent& e);
        bool OnKeyRelease(Slayer::KeyReleaseEvent& e);
        bool OnMouseMove(Slayer::MouseMoveEvent& e);
        void Update(float dt);

        inline bool GetFirstMouse() const { return m_firstMouse; }
        inline void SetFirstMouse(bool firstMouse) { m_firstMouse = firstMouse; }
    };

}