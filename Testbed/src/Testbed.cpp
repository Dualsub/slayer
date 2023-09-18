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

        Slayer::YamlDeserializer deserializer;
        deserializer.Deserialize(m_store, assetPath + "scene.yml");

        const int32_t numEntities = SL_MAX_INSTANCES;
        const int32_t side = (int32_t)std::sqrt(numEntities);
        for (int32_t i = 0; i < side; i++)
        {
            for (int32_t j = 0; j < side; j++)
            {
                float x = (i - side / 2.0f) * 115.0f;
                float z = (j - side / 2.0f) * 115.0f;
                Slayer::Entity entity = m_store.CreateEntity();
                m_store.AddComponent(entity, Slayer::Transform(Slayer::Vec3(x, 0, z), Slayer::Quat(Slayer::Vec3(-glm::pi<float>() / 2, 0, 0)), Slayer::Vec3(1.0f)));
                m_store.AddComponent(entity, Slayer::SkeletalRenderer(17529307428130246956, 5244792205592968665));
                m_store.AddComponent(entity, Slayer::AnimationPlayer({ Slayer::AnimationPlayer::AnimationClip(2688833756906273549, 0.0f, 0.5f), Slayer::AnimationPlayer::AnimationClip(85354664630225812, 0.0f, 0.5f) }));
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
