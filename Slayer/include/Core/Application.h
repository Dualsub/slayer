#pragma once

#include "Core/Core.h"
#include "Core/Containers.h"
#include "Core/Window.h"
#include "Core/Log.h"
#include "Core/Layer.h"
#include "GameTypesDecl.h"

#include <chrono>

namespace Slayer
{
    class Application
    {
        friend class Engine;
    private:
        Vector<Unique<Layer>> m_layers;

        static Application* s_instance;
        static void SetInstance(Application* app) { s_instance = app; }
    protected:
        bool m_shouldRestart = false;
        bool m_running = true;
        std::chrono::steady_clock::time_point m_lastFrameTime = std::chrono::high_resolution_clock::now();
        Timespan m_deltaTime = 0.0f;

        std::chrono::steady_clock::time_point m_lastFixedTime = std::chrono::high_resolution_clock::now();
        const Timespan m_fixedDeltaTime = 1 / 60.0f;

        Window m_window;
    public:

        static Application* Get() { return s_instance; }
        template <typename T>
        static T* Get() { return static_cast<T*>(s_instance); }

        Application() = default;
        virtual ~Application() = default;

        template<typename T, typename... Args>
        void PushLayer(Args&&... args)
        {
            m_layers.emplace_back(MakeUnique<T>(std::forward<Args>(args)...));
            m_layers.back()->OnAttach();
        }

        void PreInitialize();
        bool Initialize();
        void Update();
        void Render();
        void Shutdown();
        void HandleEvent(Event& e);

        virtual void OnPreInitialize() = 0;
        virtual void OnInitialize() = 0;
        virtual void OnUpdate(Timespan ts) = 0;
        virtual void OnFixedUpdate(Timespan ts) = 0;
        virtual void OnRender() = 0;
        virtual void OnShutdown() = 0;
        virtual void OnEvent(Event& e) = 0;

        void Stop() { m_running = false; }
        void Restart() { m_running = false; m_shouldRestart = true; }

        bool ShouldRestart() const { return m_shouldRestart; }
        void ResetRestart() { m_shouldRestart = false; }
        bool IsRunning() const { return m_running; }

        Timespan GetDeltaTime() const { return m_deltaTime; }
        void CalculateDeltaTime();
        bool CalculateFixedDeltaTime();

        Window& GetWindow() { return m_window; }
    };
}