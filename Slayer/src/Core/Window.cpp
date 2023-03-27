#include "glad/glad.h"
#include "Core/Window.h"
#include "Core/Events.h"
#include "Core/Log.h"

namespace Slayer {

    void Window::Initialize(const std::string& title, uint32_t width, uint32_t height)
    {
        SL_ASSERT(glfwInit() && "Failed to initialize GLFW");

        m_nativeHandle = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        SL_ASSERT(m_nativeHandle && "Failed to create GLFW window");

        glfwMakeContextCurrent(m_nativeHandle);

        // Initialize GLAD
        SL_ASSERT(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) && "Failed to initialize GLAD");

        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        m_width = width;
        m_height = height;

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

        SetVSync(false);

        glfwSetWindowUserPointer(m_nativeHandle, this);

		glfwSetWindowSizeCallback(m_nativeHandle, [](GLFWwindow* window, int width, int height) {
			Window* user = (Window*)glfwGetWindowUserPointer(window);
			user->m_width = width;
			user->m_height = height;
			WindowResizeEvent e(width, height);
			user->m_eventCallback(e);
			Log::Info("Window resized to", width, height);
		});

		glfwSetFramebufferSizeCallback(m_nativeHandle, [](GLFWwindow* window, int width, int height) {
			Log::Info("Framebuffer resized to", width, height);
			glViewport(0, 0, width, height);
		});
		
		glfwSetKeyCallback(m_nativeHandle, [](GLFWwindow* window, int key, int scancode, int action, int mode) {

			Window* user = (Window*)glfwGetWindowUserPointer(window);
			switch (action)
			{
			case GLFW_PRESS:
			{
				KeyPressEvent e(user->GetKey(key));
				user->m_eventCallback(e);
				break;
			}
			case GLFW_RELEASE:
			{
				KeyReleaseEvent e(user->GetKey(key));
				user->m_eventCallback(e);
				break;
			}
			default:
				break;
			}
		});

		// glfwSetMouseButtonCallback(m_nativeHandle, [](GLFWwindow* window, int button, int action, int mods)
		// {
		// 	Window* user = (Window*)glfwGetWindowUserPointer(window);
		// 	switch (action)
		// 	{
		// 	case GLFW_PRESS:
		// 	{
		// 		MouseButtonPressEvent e(user->GetButton(button));
		// 		user->m_eventCallback(e);
		// 		break;
		// 	}
		// 	case GLFW_RELEASE:
		// 	{
		// 		MouseButtonReleaseEvent e(user->GetButton(button));
		// 		user->m_eventCallback(e);
		// 		break;
		// 	}
		// 	default:
		// 		break;

		// 	}
		// });

		glfwSetCursorPosCallback(m_nativeHandle, [](GLFWwindow* window, double xpos, double ypos) {
			Window* user = (Window*)glfwGetWindowUserPointer(window);
			MouseMoveEvent e((int)xpos, (int)ypos);
			user->m_eventCallback(e);
		});

		SetCursorEnabled(false);
    }

    void Window::Shutdown()
    {
        glfwDestroyWindow(m_nativeHandle);
        glfwTerminate();
    }

    bool Window::ShouldClose()
    {
        return glfwWindowShouldClose(m_nativeHandle);
    }

    void Window::PollEvents()
    {
        glfwPollEvents();
    }

    void Window::SwapBuffers()
    {
        glfwSwapBuffers(m_nativeHandle);
    }

    void Window::SetVSync(bool enabled)
    {
        if (enabled)
            glfwSwapInterval(1);
        else
            glfwSwapInterval(0);

        m_VSync = enabled;
    }

}