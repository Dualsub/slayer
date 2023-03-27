#pragma once

#include "Core/Application.h"
#include "Core/Log.h"
#include "Core/Window.h"
#include "Core/Math.h"
#include "Core/Events.h"

#include "Rendering/Renderer/Renderer.h"
#include "Rendering/RenderingSystem.h"

#include "Scene/Components.h"
#include "Scene/ComponentStore.h"
#include "Resources/AssetPack.h"
#include "Resources/ResourceManager.h"
#include "Serialization/SceneSerializer.h"
#include "Serialization/YamlSerializer.h"

#include"Rendering/Renderer/Renderer.h"
#include "SandboxCamera.h"

#include <iostream>
#include <thread>
#include <future>
#include <iomanip>
#include <sstream>


namespace Testbed
{
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

        Slayer::Window m_window;

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

        virtual bool Initialize() override
        {
            InitializeResources();
            InitializeWindow();
            InitializeScene();
            InitializeGUI();

            return true;
        }

        virtual void Update() override
        {
            CalculateDeltaTime();
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
                m_camera->Update(m_deltaTime);
                m_renderingSystem.Update(m_deltaTime, m_store);

                m_renderer.Clear();
                m_renderer.BeginScene({}, Slayer::DirectionalLight(Slayer::Vec3(-1.0f), Slayer::Vec3(1.0f)));

                m_renderingSystem.Render(m_renderer, m_store);

                m_renderer.DrawShadows();
                m_renderer.Draw();
                m_renderer.DrawLines();

                m_renderer.EndScene();
                break;
            }
            case AS_Quitting:
                Stop();
                break;
            }

            UpdateGUI();
        }

        virtual void Render() override
        {

            m_window.SwapBuffers();
        }

        virtual void Shutdown() override
        {
            Slayer::ResourceManager::Shutdown();
            ShutdownGUI();
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

        Slayer::Window& GetWindow() { return m_window; }
    };
}