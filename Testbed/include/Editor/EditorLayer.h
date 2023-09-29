#pragma once

#include "Slayer.h"
#include "Core/Events.h"
#include "Scene/ComponentStore.h"
#include "Editor/PropertySerializer.h"

namespace Slayer::Editor {

    struct EditorSettings
    {
        std::string lastScenePath;

        template<typename Serializer>
        void Transfer(Serializer& serializer)
        {
            SL_TRANSFER_VAR(lastScenePath);
        }
    };

    class EditorLayer : public Layer
    {
    private:
        // Settings
        EditorSettings m_settings;

        // Runtime state
        bool m_editMode = false;
        Timespan m_deltaTime;
        Entity m_selectedEntity = SL_INVALID_ENTITY;
        PropertySerializer m_propertySerializer;

        // Loading
        bool m_loadingScene = false;
        std::future<void> m_loadSceneFuture;
        ComponentStore m_loadSceneStore;

        void InitializeGUI();
        void UpdateGUI();
        void ShutdownGUI();

        void ToggleEditMode();

        void SaveScene(ComponentStore& store, const std::string& path);
        void LoadScene(const std::string& path);
        void NewScene();

        // Load Settings
        void LoadSettings();
        void SaveSettings();
    public:
        EditorLayer();
        ~EditorLayer();

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnUpdate(Timespan ts) override;
        virtual void OnRender() override;
        virtual void OnEvent(Event& e) override;

        bool OnKeyPress(KeyPressEvent& e);
        bool OnMouseMove(MouseMoveEvent& e);
    };

}
