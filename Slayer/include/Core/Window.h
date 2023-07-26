#pragma once 

#include "Core/Core.h"
#include "Core/Event.h"
#include "Input/Keys.h"

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3native.h"
#include "GLFW/glfw3.h"
#include "glad/glad.h"
#include <functional>

namespace Slayer
{
    /// GLFW implmentation of a window
    class Window
    {
    public:
        using WindowResizeCallback = std::function<void(uint32_t, uint32_t)>;

        Window()
        {
            m_keyMap.Insert(SlayerKey::KEY_UNKNOWN, GLFW_KEY_UNKNOWN);
            m_keyMap.Insert(SlayerKey::KEY_SPACE, GLFW_KEY_SPACE);
            m_keyMap.Insert(SlayerKey::KEY_APOSTROPHE, GLFW_KEY_APOSTROPHE);
            m_keyMap.Insert(SlayerKey::KEY_COMMA, GLFW_KEY_COMMA);
            m_keyMap.Insert(SlayerKey::KEY_MINUS, GLFW_KEY_MINUS);
            m_keyMap.Insert(SlayerKey::KEY_PERIOD, GLFW_KEY_PERIOD);
            m_keyMap.Insert(SlayerKey::KEY_SLASH, GLFW_KEY_SLASH);
            m_keyMap.Insert(SlayerKey::KEY_0, GLFW_KEY_0);
            m_keyMap.Insert(SlayerKey::KEY_1, GLFW_KEY_1);
            m_keyMap.Insert(SlayerKey::KEY_2, GLFW_KEY_2);
            m_keyMap.Insert(SlayerKey::KEY_3, GLFW_KEY_3);
            m_keyMap.Insert(SlayerKey::KEY_4, GLFW_KEY_4);
            m_keyMap.Insert(SlayerKey::KEY_5, GLFW_KEY_5);
            m_keyMap.Insert(SlayerKey::KEY_6, GLFW_KEY_6);
            m_keyMap.Insert(SlayerKey::KEY_7, GLFW_KEY_7);
            m_keyMap.Insert(SlayerKey::KEY_8, GLFW_KEY_8);
            m_keyMap.Insert(SlayerKey::KEY_9, GLFW_KEY_9);
            m_keyMap.Insert(SlayerKey::KEY_SEMICOLON, GLFW_KEY_SEMICOLON);
            m_keyMap.Insert(SlayerKey::KEY_EQUAL, GLFW_KEY_EQUAL);
            m_keyMap.Insert(SlayerKey::KEY_A, GLFW_KEY_A);
            m_keyMap.Insert(SlayerKey::KEY_B, GLFW_KEY_B);
            m_keyMap.Insert(SlayerKey::KEY_C, GLFW_KEY_C);
            m_keyMap.Insert(SlayerKey::KEY_D, GLFW_KEY_D);
            m_keyMap.Insert(SlayerKey::KEY_E, GLFW_KEY_E);
            m_keyMap.Insert(SlayerKey::KEY_F, GLFW_KEY_F);
            m_keyMap.Insert(SlayerKey::KEY_G, GLFW_KEY_G);
            m_keyMap.Insert(SlayerKey::KEY_H, GLFW_KEY_H);
            m_keyMap.Insert(SlayerKey::KEY_I, GLFW_KEY_I);
            m_keyMap.Insert(SlayerKey::KEY_J, GLFW_KEY_J);
            m_keyMap.Insert(SlayerKey::KEY_K, GLFW_KEY_K);
            m_keyMap.Insert(SlayerKey::KEY_L, GLFW_KEY_L);
            m_keyMap.Insert(SlayerKey::KEY_M, GLFW_KEY_M);
            m_keyMap.Insert(SlayerKey::KEY_N, GLFW_KEY_N);
            m_keyMap.Insert(SlayerKey::KEY_O, GLFW_KEY_O);
            m_keyMap.Insert(SlayerKey::KEY_P, GLFW_KEY_P);
            m_keyMap.Insert(SlayerKey::KEY_Q, GLFW_KEY_Q);
            m_keyMap.Insert(SlayerKey::KEY_R, GLFW_KEY_R);
            m_keyMap.Insert(SlayerKey::KEY_S, GLFW_KEY_S);
            m_keyMap.Insert(SlayerKey::KEY_T, GLFW_KEY_T);
            m_keyMap.Insert(SlayerKey::KEY_U, GLFW_KEY_U);
            m_keyMap.Insert(SlayerKey::KEY_V, GLFW_KEY_V);
            m_keyMap.Insert(SlayerKey::KEY_W, GLFW_KEY_W);
            m_keyMap.Insert(SlayerKey::KEY_X, GLFW_KEY_X);
            m_keyMap.Insert(SlayerKey::KEY_Y, GLFW_KEY_Y);
            m_keyMap.Insert(SlayerKey::KEY_Z, GLFW_KEY_Z);
            m_keyMap.Insert(SlayerKey::KEY_LEFT_BRACKET, GLFW_KEY_LEFT_BRACKET);
            m_keyMap.Insert(SlayerKey::KEY_BACKSLASH, GLFW_KEY_BACKSLASH);
            m_keyMap.Insert(SlayerKey::KEY_RIGHT_BRACKET, GLFW_KEY_RIGHT_BRACKET);
            m_keyMap.Insert(SlayerKey::KEY_GRAVE_ACCENT, GLFW_KEY_GRAVE_ACCENT);
            m_keyMap.Insert(SlayerKey::KEY_WORLD_1, GLFW_KEY_WORLD_1);
            m_keyMap.Insert(SlayerKey::KEY_WORLD_2, GLFW_KEY_WORLD_2);
            m_keyMap.Insert(SlayerKey::KEY_ESCAPE, GLFW_KEY_ESCAPE);
            m_keyMap.Insert(SlayerKey::KEY_ENTER, GLFW_KEY_ENTER);
            m_keyMap.Insert(SlayerKey::KEY_TAB, GLFW_KEY_TAB);
            m_keyMap.Insert(SlayerKey::KEY_BACKSPACE, GLFW_KEY_BACKSPACE);
            m_keyMap.Insert(SlayerKey::KEY_INSERT, GLFW_KEY_INSERT);
            m_keyMap.Insert(SlayerKey::KEY_DELETE, GLFW_KEY_DELETE);
            m_keyMap.Insert(SlayerKey::KEY_RIGHT, GLFW_KEY_RIGHT);
            m_keyMap.Insert(SlayerKey::KEY_LEFT, GLFW_KEY_LEFT);
            m_keyMap.Insert(SlayerKey::KEY_DOWN, GLFW_KEY_DOWN);
            m_keyMap.Insert(SlayerKey::KEY_UP, GLFW_KEY_UP);
            m_keyMap.Insert(SlayerKey::KEY_PAGE_UP, GLFW_KEY_PAGE_UP);
            m_keyMap.Insert(SlayerKey::KEY_PAGE_DOWN, GLFW_KEY_PAGE_DOWN);
            m_keyMap.Insert(SlayerKey::KEY_HOME, GLFW_KEY_HOME);
            m_keyMap.Insert(SlayerKey::KEY_END, GLFW_KEY_END);
            m_keyMap.Insert(SlayerKey::KEY_CAPS_LOCK, GLFW_KEY_CAPS_LOCK);
            m_keyMap.Insert(SlayerKey::KEY_SCROLL_LOCK, GLFW_KEY_SCROLL_LOCK);
            m_keyMap.Insert(SlayerKey::KEY_NUM_LOCK, GLFW_KEY_NUM_LOCK);
            m_keyMap.Insert(SlayerKey::KEY_PRINT_SCREEN, GLFW_KEY_PRINT_SCREEN);
            m_keyMap.Insert(SlayerKey::KEY_PAUSE, GLFW_KEY_PAUSE);
            m_keyMap.Insert(SlayerKey::KEY_F1, GLFW_KEY_F1);
            m_keyMap.Insert(SlayerKey::KEY_F2, GLFW_KEY_F2);
            m_keyMap.Insert(SlayerKey::KEY_F3, GLFW_KEY_F3);
            m_keyMap.Insert(SlayerKey::KEY_F4, GLFW_KEY_F4);
            m_keyMap.Insert(SlayerKey::KEY_F5, GLFW_KEY_F5);
            m_keyMap.Insert(SlayerKey::KEY_F6, GLFW_KEY_F6);
            m_keyMap.Insert(SlayerKey::KEY_F7, GLFW_KEY_F7);
            m_keyMap.Insert(SlayerKey::KEY_F8, GLFW_KEY_F8);
            m_keyMap.Insert(SlayerKey::KEY_F9, GLFW_KEY_F9);
            m_keyMap.Insert(SlayerKey::KEY_F10, GLFW_KEY_F10);
            m_keyMap.Insert(SlayerKey::KEY_F11, GLFW_KEY_F11);
            m_keyMap.Insert(SlayerKey::KEY_F12, GLFW_KEY_F12);
            m_keyMap.Insert(SlayerKey::KEY_F13, GLFW_KEY_F13);
            m_keyMap.Insert(SlayerKey::KEY_F14, GLFW_KEY_F14);
            m_keyMap.Insert(SlayerKey::KEY_F15, GLFW_KEY_F15);
            m_keyMap.Insert(SlayerKey::KEY_F16, GLFW_KEY_F16);
            m_keyMap.Insert(SlayerKey::KEY_F17, GLFW_KEY_F17);
            m_keyMap.Insert(SlayerKey::KEY_F18, GLFW_KEY_F18);
            m_keyMap.Insert(SlayerKey::KEY_F19, GLFW_KEY_F19);
            m_keyMap.Insert(SlayerKey::KEY_F20, GLFW_KEY_F20);
            m_keyMap.Insert(SlayerKey::KEY_F21, GLFW_KEY_F21);
            m_keyMap.Insert(SlayerKey::KEY_F22, GLFW_KEY_F22);
            m_keyMap.Insert(SlayerKey::KEY_F23, GLFW_KEY_F23);
            m_keyMap.Insert(SlayerKey::KEY_F24, GLFW_KEY_F24);
            m_keyMap.Insert(SlayerKey::KEY_F25, GLFW_KEY_F25);
            m_keyMap.Insert(SlayerKey::KEY_KP_0, GLFW_KEY_KP_0);
            m_keyMap.Insert(SlayerKey::KEY_KP_1, GLFW_KEY_KP_1);
            m_keyMap.Insert(SlayerKey::KEY_KP_2, GLFW_KEY_KP_2);
            m_keyMap.Insert(SlayerKey::KEY_KP_3, GLFW_KEY_KP_3);
            m_keyMap.Insert(SlayerKey::KEY_KP_4, GLFW_KEY_KP_4);
            m_keyMap.Insert(SlayerKey::KEY_KP_5, GLFW_KEY_KP_5);
            m_keyMap.Insert(SlayerKey::KEY_KP_6, GLFW_KEY_KP_6);
            m_keyMap.Insert(SlayerKey::KEY_KP_7, GLFW_KEY_KP_7);
            m_keyMap.Insert(SlayerKey::KEY_KP_8, GLFW_KEY_KP_8);
            m_keyMap.Insert(SlayerKey::KEY_KP_9, GLFW_KEY_KP_9);
            m_keyMap.Insert(SlayerKey::KEY_KP_DECIMAL, GLFW_KEY_KP_DECIMAL);
            m_keyMap.Insert(SlayerKey::KEY_KP_DIVIDE, GLFW_KEY_KP_DIVIDE);
            m_keyMap.Insert(SlayerKey::KEY_KP_MULTIPLY, GLFW_KEY_KP_MULTIPLY);
            m_keyMap.Insert(SlayerKey::KEY_KP_SUBTRACT, GLFW_KEY_KP_SUBTRACT);
            m_keyMap.Insert(SlayerKey::KEY_KP_ADD, GLFW_KEY_KP_ADD);
            m_keyMap.Insert(SlayerKey::KEY_KP_ENTER, GLFW_KEY_KP_ENTER);
            m_keyMap.Insert(SlayerKey::KEY_KP_EQUAL, GLFW_KEY_KP_EQUAL);
            m_keyMap.Insert(SlayerKey::KEY_LEFT_SHIFT, GLFW_KEY_LEFT_SHIFT);
            m_keyMap.Insert(SlayerKey::KEY_LEFT_CONTROL, GLFW_KEY_LEFT_CONTROL);
            m_keyMap.Insert(SlayerKey::KEY_LEFT_ALT, GLFW_KEY_LEFT_ALT);
            m_keyMap.Insert(SlayerKey::KEY_LEFT_SUPER, GLFW_KEY_LEFT_SUPER);
            m_keyMap.Insert(SlayerKey::KEY_RIGHT_SHIFT, GLFW_KEY_RIGHT_SHIFT);
            m_keyMap.Insert(SlayerKey::KEY_RIGHT_CONTROL, GLFW_KEY_RIGHT_CONTROL);
            m_keyMap.Insert(SlayerKey::KEY_RIGHT_ALT, GLFW_KEY_RIGHT_ALT);
            m_keyMap.Insert(SlayerKey::KEY_RIGHT_SUPER, GLFW_KEY_RIGHT_SUPER);
            m_keyMap.Insert(SlayerKey::KEY_MENU, GLFW_KEY_MENU);
        }
        ~Window() = default;

        bool IsOpen() const { return !glfwWindowShouldClose(m_nativeHandle); }
        void Close() { glfwSetWindowShouldClose(m_nativeHandle, true); }

        void SetVSync(bool enabled);
        bool IsVSync() const { return m_VSync; }

        void SetTitle(const char* title) { glfwSetWindowTitle(m_nativeHandle, title); }
        void SetCursorEnabled(bool enabled) { glfwSetInputMode(m_nativeHandle, GLFW_CURSOR, enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED); }

        void SetEventCallback(const EventCallback& callback) { m_eventCallback = callback; }

        void* GetNativeWindow() const { return m_nativeHandle; }

        void Initialize(const std::string& title, uint32_t width, uint32_t height);
        void Shutdown();

        bool ShouldClose();
        void PollEvents();
        void SwapBuffers();
        void MakeContextCurrent() { glfwMakeContextCurrent(m_nativeHandle); }

        void BindWindowResizeCallback(const WindowResizeCallback& callback) { m_windowResizeCallback = callback; }
        const WindowResizeCallback& GetWindowResizeCallback() const { return m_windowResizeCallback; }

        uint32_t GetWidth() const { return m_width; }
        uint32_t GetHeight() const { return m_height; }

        SlayerKey GetKey(uint32_t key)
        {
            return m_keyMap.Get(key);
        }

        uint32_t FromKey(SlayerKey key)
        {
            return m_keyMap.From(key);
        }

    private:
        void SetSize(uint32_t width, uint32_t height)
        {
            m_width = width;
            m_height = height;
        }

        GLFWwindow* m_nativeHandle;
        bool m_VSync;
        uint32_t m_width, m_height;

        EventCallback m_eventCallback;

        WindowResizeCallback m_windowResizeCallback;

        KeyMap<uint32_t> m_keyMap;
    };

}