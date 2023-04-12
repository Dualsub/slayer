#pragma once

#include "Slayer.h"
#include "Core/Events.h"

namespace Slayer {

    class EditorLayer : public Layer
    {
    private:
        Timespan m_deltaTime;

        void InitializeGUI();
        void UpdateGUI();
        void ShutdownGUI();
    public:
        EditorLayer();
        ~EditorLayer();

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnUpdate(Timespan ts) override;
        virtual void OnRender() override;
        virtual void OnEvent(Event& e) override;
        
        bool OnKeyPress(KeyPressEvent& e);
    };

}
