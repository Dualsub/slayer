#pragma once

#include "Core/Application.h"
#include "Core/Layer.h"
#include "Core/Log.h"
#include "Core/Window.h"
#include "Core/Math.h"
#include "Core/Events.h"

#include "Rendering/Renderer/Renderer.h"
#include "Rendering/RenderingSystem.h"
#include "Rendering/Animation/AnimationSystem.h"

#include "Scene/TransformSystem.h"
#include "Scene/Components.h"
#include "Scene/ComponentStore.h"
#include "Resources/AssetPack.h"
#include "Resources/ResourceManager.h"
#include "Serialization/SceneSerializer.h"
#include "Serialization/YamlSerializer.h"

#include "Editor/EditorLayer.h"
#include "SandboxCamera.h"

#include <iostream>
#include <thread>
#include <future>
#include <iomanip>
#include <sstream>

namespace Testbed
{
    class TestbedLayer: public Slayer::Layer
    {
    public:
        TestbedLayer() = default;
        ~TestbedLayer() = default;

        virtual void OnAttach()
        {

        }

        virtual void OnDetach()
        {

        }

        virtual void OnUpdate(Slayer::Timespan ts)
        {

        }

        virtual void OnRender()
        {

        }

        virtual void OnEvent(Slayer::Event& e)
        {

        }
    };

    enum ApplicationState
    {
        AS_Loading,
        AS_Running,
        AS_Quitting
    };

    class TestbedApplication: public Slayer::Application
    {
    private:
        ApplicationState m_state = AS_Running;

        std::future<Slayer::GPULoadData> m_levelLoadFuture;
        Slayer::AssetPack m_assetPack;

        Slayer::ComponentStore m_store;

        Slayer::Shared<SandboxCamera> m_camera;
        Slayer::Renderer m_renderer;
        Slayer::RenderingSystem m_renderingSystem;
        Slayer::AnimationSystem m_animationSystem;
        Slayer::TransformSystem m_transformSystem;

        template<typename T>
        bool LoadScene(std::future<T>& future)
        {
            return true;
        }

        void InitializeResources();
        void InitializeScene();
        void InitializeRendering();
        void InitializeWindow();
        void InitializeGUI();
        void UpdateGUI();
        void ShutdownGUI();
        void ShutdownRendering();

        std::future<void> SaveScene(const std::string& filename, Slayer::ComponentStore& store)
        {
            return std::async(std::launch::async, [filename, &store]() {
                Slayer::YamlSerializer serializer;
                serializer.Serialize(store, filename);
                });
        }

        std::future<void> LoadScene(const std::string& filename, Slayer::ComponentStore& store)
        {
            return std::async(std::launch::async, [filename, &store]() {
                Slayer::YamlDeserializer deserializer;
                deserializer.Deserialize(store, filename);
                });
        }

    public:
        TestbedApplication() = default;
        ~TestbedApplication() = default;

        virtual void OnPreInitialize() override
        {

        }

        virtual void OnInitialize() override
        {
            InitializeResources();
            InitializeWindow();
            InitializeScene();

            PushLayer<class TestbedLayer>();
            PushLayer<Slayer::EditorLayer>();
        }

        virtual void OnUpdate(Slayer::Timespan ts) override
        {
            m_window.PollEvents();
            if (m_window.ShouldClose())
                m_state = AS_Quitting;

            switch (m_state)
            {
            case AS_Loading:
            {
                if (m_levelLoadFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
                {
                    Slayer::ResourceManager* rm = Slayer::ResourceManager::Get();
                    Slayer::GPULoadData gpuLoadData = m_levelLoadFuture.get();
                    rm->LoadGPUAssets(gpuLoadData);
                    InitializeRendering();
                    m_state = AS_Running;
                }
                else
                {
                    // Draw loading screen
                }

                break;
            }
            case AS_Running:
                // Update game
            {
                m_camera->Update(ts);
                m_animationSystem.Update(ts, m_store);
                m_transformSystem.Update(ts, m_store);
                m_renderingSystem.Update(ts, m_store);
                break;
            }
            case AS_Quitting:
                Stop(); 
                break;
            }
        }

        virtual void OnRender() override
        {
            m_renderer.Clear();

            if (m_state != AS_Running)
                return;

            m_renderer.BeginScene({}, Slayer::DirectionalLight(Slayer::Vec3(-1.0f), Slayer::Vec3(1.0f)));

            m_renderingSystem.Render(m_renderer, m_store);

            m_renderer.DrawShadows();
            m_renderer.Draw();
            m_renderer.DrawLines();

            m_renderer.EndScene();
        }

        virtual void OnShutdown() override
        {
            Slayer::ResourceManager::Shutdown();
            m_window.Shutdown();
        }

        void OnEvent(Slayer::Event& e)
        {
            if (m_camera)
                m_camera->OnEvent(e);

            SL_EVENT_DISPATCH(Slayer::WindowResizeEvent, Testbed::TestbedApplication::OnResize);
        }

        bool OnResize(Slayer::WindowResizeEvent& e)
        {
            Slayer::Log::Info("Window resized to:", e.width, e.height);
            m_renderer.Resize(e.width, e.height);
            return true;
        }
    };
}