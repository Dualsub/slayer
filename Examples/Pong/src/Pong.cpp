#include "Pong.h"
#include "Input/Input.h"
#include "GameTypesDecl.h"

namespace Pong
{
    const std::string assetPath = "Examples/Pong/assets/";

    void PongApplication::InitializeResources()
    {
        Slayer::ResourceManager::Initialize();
        Slayer::ResourceManager* rm = Slayer::ResourceManager::Get();

        m_levelLoadFuture = rm->LoadAssetsAsync(assetPath + "pack.slp");
        m_state = AS_Loading;
    }

    void PongApplication::InitializeScene()
    {
        Slayer::ForEachComponentType([this]<typename T>() {
            m_store.RegisterComponent<T>();
        });

        Slayer::YamlDeserializer deserializer;
        deserializer.Deserialize(m_store, assetPath + "scene.yml");
    }

    void PongApplication::InitializeRendering()
    {
        m_camera = Slayer::MakeShared<SandboxCamera>(100.0f);
        m_renderer.Initialize(m_camera, m_window.GetWidth(), m_window.GetHeight());
    }

    void PongApplication::ShutdownRendering()
    {
        m_renderer.CleanUp();
    }

    void PongApplication::InitializeWindow()
    {
        m_window.Initialize("Pong", 1600, 900);
        m_window.SetEventCallback(SL_EVENT_BIND(Application::HandleEvent));

        Slayer::Input::Initialize(&m_window);
    }


    void PongApplication::OnGameUpdate(Slayer::Timespan ts, Slayer::ComponentStore& store)
    {
        Slayer::Entity ballEntity = store.FindFirst<Ball>();
        if (ballEntity == SL_INVALID_ENTITY)
            return;
        Ball* ball = store.GetComponent<Ball>(ballEntity);

        // Update paddle positions
        store.ForEach<Paddle, Slayer::Transform>([&ts](Slayer::Entity entity, Paddle* paddle, Slayer::Transform* transform) {
            transform->position.y += paddle->velocity * ts;
            });
    };
}

Slayer::Application* RegisterApplication(int argc, const char** argv)
{
    return new Pong::PongApplication();
}
