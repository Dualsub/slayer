#include "Core/Application.h"

namespace Slayer
{
    Application* Application::s_instance = nullptr;

    void Application::PreInitialize()
    {
        std::cout << "Application::PreInitialize()" << std::endl;
        OnPreInitialize();
    }

    // Calculate the delta time between the last frame and the current frame in seconds
    void Application::CalculateDeltaTime()
    {
        auto ts = std::chrono::high_resolution_clock::now();
        m_deltaTime = std::chrono::duration_cast<std::chrono::nanoseconds>(ts - m_lastFrameTime).count() / 1000000000.0f;
        m_lastFrameTime = ts;
        return;
    }

    void Application::Update()
    {
        CalculateDeltaTime();
        OnUpdate(m_deltaTime);
        for (auto& layer : m_layers)
            layer->OnUpdate(m_deltaTime);
    }

    void Application::Render()
    {
        OnRender();
        for (auto& layer : m_layers)
            layer->OnRender();

        m_window.SwapBuffers();
    }

    void Application::Shutdown()
    {
        OnShutdown();
    }

    bool Application::Initialize()
    {
        OnInitialize();
        return true;
    }

    void Application::HandleEvent(Event& e)
    {
        OnEvent(e);
        for (size_t i = m_layers.size() - 1; i > 0; i--)
            m_layers[i]->OnEvent(e);
    }

}