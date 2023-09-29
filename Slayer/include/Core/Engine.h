#pragma once

#include "Core/Log.h"
#include "Core/Application.h"

namespace Slayer
{
    class Engine
    {
    private:
        bool Initialize();
        void Shutdown();

        template<typename Manager_t, typename ...VArgs>
        bool InitializeManager(VArgs... args)
        {
            Manager_t* m = new Manager_t();
            if (!m || !m->Initialize())
            {
                Log::Critical("Failed to initialize", typeid(Manager_t).name());
                delete m;
                return false;
            }

            return true;
        }

        template<typename Manager_t>
        void ShutdownManager()
        {
            Manager_t* m = Manager_t::Get();
            if (m)
            {
                m->Shutdown();
            }

            delete m;
        }

    public:
        Engine() = default;
        ~Engine() = default;

        void RunMainLoop(Application* app);
    };

}