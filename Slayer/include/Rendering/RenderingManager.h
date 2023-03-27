#pragma once

namespace Slayer
{
    class RenderingManager
    {
    friend class Engine;
    private:
        static RenderingManager *s_instance;
    public:
        RenderingManager() = default;
        ~RenderingManager() = default;

        bool Initialize();
        void Shutdown();

        void Render();

        static RenderingManager *Get() { return s_instance; }
    };
}