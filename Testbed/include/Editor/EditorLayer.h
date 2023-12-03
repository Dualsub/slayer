#pragma once

#include "Core/Core.h"
#include "Core/Events.h"
#include "Scene/ComponentStore.h"
#include "Editor/PropertySerializer.h"
#include "Editor/EditorCamera.h"

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

    struct SelectionContext
    {
        Entity entity = SL_INVALID_ENTITY;
        ComponentType singleton = 0;

        SelectionContext() = default;
        SelectionContext(Entity entity, ComponentType singleton) :
            entity(entity), singleton(singleton)
        {
        }
        ~SelectionContext() = default;

        void SelectEntity(Entity entity)
        {
            this->entity = entity;
            singleton = 0;
        }

        void SelectSingleton(ComponentType singleton)
        {
            entity = SL_INVALID_ENTITY;
            this->singleton = singleton;
        }
    };

    class EditorLayer : public Layer
    {
    private:
        std::string m_assetDirectory;

        // Settings
        EditorSettings m_settings;

        // Camera
        EditorCamera m_camera;

        // Runtime state
        bool m_editMode = false;
        bool m_renderColliders = false;
        Timespan m_deltaTime;
        SelectionContext m_selection;
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
        EditorLayer(const std::string& assetDirectory);
        ~EditorLayer();

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnUpdate(Timespan ts) override;
        virtual void OnFixedUpdate(Timespan ts) override {}
        virtual void OnRender() override;
        virtual void OnEvent(Event& e) override;

        const std::string& GetAssetDirectory() const { return m_assetDirectory; }

        bool OnKeyPress(KeyPressEvent& e);
        bool OnMouseMove(MouseMoveEvent& e);
        bool OnWindowResize(WindowResizeEvent& e);
    };

}
