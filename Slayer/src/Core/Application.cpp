#include "Core/Application.h"

namespace Slayer
{
    Application* Application::s_instance = nullptr;

    void Application::PreInitialize()
    {
        std::cout << "Application::PreInitialize()" << std::endl;
    }

    // Calculate the delta time between the last frame and the current frame in seconds
    void Application::CalculateDeltaTime()
    {
        auto ts = std::chrono::high_resolution_clock::now();
        m_deltaTime = std::chrono::duration_cast<std::chrono::nanoseconds>(ts - m_lastFrameTime).count() / 1000000000.0f;
        m_lastFrameTime = ts;
        return;
    }
}