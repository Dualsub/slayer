#include "Input/Input.h"
#include "glfw/glfw3.h"

namespace Slayer {

    Window* Input::m_window = nullptr;

    void Input::Initialize(Window* window)
    {
        m_window = window;
    }

    void Input::Shutdown()
    {
        m_window = nullptr;
    }

    bool Input::IsKeyPressed(SlayerKey key)
    {
        SL_ASSERT(m_window && "Input::Initialize() must be called before using Input");
        return glfwGetKey((GLFWwindow*)m_window->GetNativeWindow(), m_window->FromKey(key)) == GLFW_PRESS;
    }

    bool Input::IsMouseButtonPressed(int button)
    {
        SL_ASSERT(m_window && "Input::Initialize() must be called before using Input");
        return glfwGetMouseButton((GLFWwindow*)m_window->GetNativeWindow(), button) == GLFW_PRESS;
    }

    Vec2 Input::GetMousePosition()
    {
        SL_ASSERT(m_window && "Input::Initialize() must be called before using Input");
        double x, y;
        glfwGetCursorPos((GLFWwindow*)m_window->GetNativeWindow(), &x, &y);
        return Vec2((float)x, (float)y);
    }
}