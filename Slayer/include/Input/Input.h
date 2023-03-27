#pragma once

#include "Core/Core.h"
#include "Core/Math.h"
#include "Input/Keys.h"
#include "Core/Window.h"

namespace Slayer {

    class Input
    {
    public:
        static Window* m_window;
        static void Initialize(Window* window);
        static void Shutdown();

        static bool IsKeyPressed(SlayerKey key);
        static bool IsMouseButtonPressed(int button);
        static Vec2 GetMousePosition();
    };
}