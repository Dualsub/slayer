#include "Testbed.h"
#include "Resources/ResourceManager.h"
#include "Input/Input.h"
#include "Scene/Components.h"
#include "Utils.h"

#include <filesystem>

const std::string assetPath = "C:/dev/repos/Slayer/Testbed/assets/";

namespace Testbed
{
    void TestbedApplication::InitializeResources()
    {
        Slayer::ResourceManager::Initialize();
        Slayer::ResourceManager* rm = Slayer::ResourceManager::Get();

        Slayer::Log::Info(std::filesystem::current_path().string());

        m_levelLoadFuture = rm->LoadAssetsAsync(assetPath + "pack.slp");
        m_state = AS_Loading;
    }

    void TestbedApplication::InitializeScene()
    {
        Slayer::ForEachComponentType([this]<typename T>() {
            m_store.RegisterComponent<T>();
        });

        // Slayer::YamlDeserializer deserializer;
        // deserializer.Deserialize(m_store, assetPath + "scene.yml");
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(-20.0f, 20.0f);

        const size_t numEntities = 128;
        const size_t side = (int)glm::sqrt(numEntities);

        for (size_t i = 0; i < side; i++)
        {
            for (size_t j = 0; j < side; j++)
            {
                Slayer::Entity e = m_store.CreateEntity();
                float x = i * 120.0f + dis(gen);
                float z = j * 120.0f + dis(gen);
                m_store.AddComponent(e, Slayer::Transform(Slayer::Vec3(x, 0.0, z), Slayer::Quat(Slayer::Vec3(-glm::pi<float>() / 2, 0, 0)), Slayer::Vec3(1.0)));
                m_store.AddComponent(e, Slayer::SkeletalRenderer(17529307428130246956, 5244792205592968665));
            }
        }
    }

    void TestbedApplication::InitializeRendering()
    {
        m_camera = Slayer::MakeShared<SandboxCamera>(100.0f);
        m_renderer.Initialize(m_camera, m_window.GetWidth(), m_window.GetHeight());
    }

    void TestbedApplication::ShutdownRendering()
    {
        m_renderer.CleanUp();
    }

    void TestbedApplication::InitializeWindow()
    {
        m_window.Initialize("Testbed", 1600, 900);
        m_window.SetEventCallback(SL_EVENT_BIND(Application::HandleEvent));

        Slayer::Input::Initialize(&m_window);
    }
}

Slayer::Application* RegisterApplication(int argc, const char** argv)
{
    return new Testbed::TestbedApplication();
}
