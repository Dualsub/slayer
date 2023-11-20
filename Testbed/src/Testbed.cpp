#include "Testbed.h"

#include "Resources/ResourceManager.h"
#include "Input/Input.h"
#include "Scene/Components.h"
#include "Scene/TransformSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/ContactSystem.h"
#include "Physics/ColliderRenderingSystem.h"
#include "Rendering/RenderingSystem.h"
#include "Rendering/Animation/AnimationSystem.h"
#include "Systems/CharacterMovementSystem.h"
#include "Systems/FiringSystem.h"
#include "Utils.h"

#include <filesystem>

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
        m_systemManager.RegisterSystem<Slayer::RenderingSystem>();
        m_systemManager.RegisterSystem<Slayer::AnimationSystem>();
        m_systemManager.RegisterSystem<Slayer::TransformSystem>();
        m_systemManager.RegisterSystem<Slayer::PhysicsSystem>();
        m_systemManager.RegisterSystem<Slayer::ContactSystem>();
        m_systemManager.RegisterSystem<Slayer::ColliderRenderingSystem>();
        m_systemManager.RegisterSystem<CharacterMovementSystem>();
        m_systemManager.RegisterSystem<FiringSystem>();

        auto& store = m_world.GetStore();

        Slayer::ForEachComponentType([&store]<typename T>() {
            store.RegisterComponent<T>();
        });
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
