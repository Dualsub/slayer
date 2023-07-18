#pragma once

#include "Rendering/Renderer/Camera.h"
#include "Core/Events.h"
#include "Core/Math.h"

namespace Pong {

    class StaticCamera : public Slayer::Camera
    {
    public:
        StaticCamera();
        Slayer::Vec3 forward;
        Slayer::Vec3 right;
        const Slayer::Vec3 up = Slayer::Vec3(0.0f, 1.0f, 0.0f);
        float fov = 90.0f;

        void UpdateProjection(uint32_t width, uint32_t height)
        {
            SetProjectionMatrix(fov, float(width), float(height), nearPlane, farPlane);
        }
    };

}