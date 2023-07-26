#include "Core/Engine.h"

#include "Slayer.h"
#include "Core/Log.h"
#include "Rendering/RenderingManager.h"

#include <string>

namespace Slayer
{
    bool Engine::Initialize()
    {
        if (!InitializeManager<RenderingManager>()) return false;

        Log::Info("Engine initialized");

        return true;
    }

    void Engine::Update()
    {
    }

    void Engine::Render()
    {
    }

    void Engine::Shutdown()
    {
        ShutdownManager<RenderingManager>();
    }

    void Engine::RunMainLoop(Application* app)
    {
        Application::SetInstance(app);
        app->PreInitialize();

        SL_ASSERT(Initialize() && "Failed to initialize engine");
        SL_ASSERT(app->Initialize() && "Failed to initialize application");

        while (app->IsRunning())
        {
            SL_FRAME("MainThread");
            app->Update();
            Update();
            app->Render();
            Render();
        }

        app->Shutdown();
        Application::SetInstance(nullptr);

        Shutdown();
    }

}