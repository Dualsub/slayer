#pragma once 

#include "Input/Keys.h"
#include "Core/Event.h"

namespace Slayer {

    class KeyPressEvent: public Event
    {
    public:
        KeyPressEvent(SlayerKey key): key(key) {}
        SlayerKey key;
        SL_EVENT_TYPE(KeyPressEvent);
    };

    class KeyReleaseEvent: public Event
    {
    public:
        KeyReleaseEvent(SlayerKey key): key(key) {}
        SlayerKey key;
        SL_EVENT_TYPE(KeyReleaseEvent);
    };

    class MouseButtonPressEvent: public Event
    {
    public:
        MouseButtonPressEvent(SlayerMouseButton button): button(button) {}
        SlayerMouseButton button;
        SL_EVENT_TYPE(MouseButtonPressEvent);
    };

    class MouseButtonReleaseEvent: public Event
    {
    public:
        MouseButtonReleaseEvent(SlayerMouseButton button): button(button) {}
        SlayerMouseButton button;
        SL_EVENT_TYPE(MouseButtonReleaseEvent);
    };

    class MouseMoveEvent: public Event
    {
    public:
        MouseMoveEvent(int posX, int posY): posX(posX), posY(posY) {}
        int posX;
        int posY;
        SL_EVENT_TYPE(MouseMoveEvent);
    };

    class WindowResizeEvent: public Event
    {
    public:
        WindowResizeEvent(int width, int height): width(width), height(height) {}
        int width;
        int height;
        SL_EVENT_TYPE(WindowResizeEvent);
    };

}