#pragma once

#include "Core/Log.h"
#include "GameTypesDecl.h"

#include <chrono>

namespace Slayer
{
    class Application
    {
        friend class Engine;
    private:
        static Application* s_instance;
        static void SetInstance(Application* app) { s_instance = app; }
    protected:
        bool m_shouldRestart = false;
        bool m_running = true;
        std::chrono::steady_clock::time_point m_lastFrameTime = std::chrono::high_resolution_clock::now();
        Timespan m_deltaTime = 0.0f;       
    public:
    
        static Application* Get() { return s_instance; }
        template <typename T>
        static T* Get() { return static_cast<T*>(s_instance); }

        Application() = default;
        virtual ~Application() = default;

        virtual void PreInitialize();
        virtual bool Initialize() = 0;
        virtual void Update() = 0;
        virtual void Render() = 0;
        virtual void Shutdown() = 0;

        void Stop() { m_running = false; }
        void Restart() { m_running = false; m_shouldRestart = true; }

        bool ShouldRestart() const { return m_shouldRestart; }
        void ResetRestart() { m_shouldRestart = false; }
        bool IsRunning() const { return m_running; }

        Timespan GetDeltaTime() const { return m_deltaTime; }
        void CalculateDeltaTime();
    };
}