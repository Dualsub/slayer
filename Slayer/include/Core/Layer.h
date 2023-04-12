#pragma once

#include "Core/Core.h"
#include "Core/Event.h"

namespace Slayer {

    class Layer
    {
    public:
        Layer();
        ~Layer();

        virtual void OnAttach() = 0;
        virtual void OnDetach() = 0;
        virtual void OnUpdate(Timespan ts) = 0;
        virtual void OnRender() = 0;
        virtual void OnEvent(Event& e) = 0;
    };

}
