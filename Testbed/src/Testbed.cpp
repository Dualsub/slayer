#include "Testbed.h"
#include "Resources/ResourceManager.h"
#include "Input/Input.h"
#include "Scene/Components.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

namespace Testbed
{
    void TestbedApplication::InitializeResources()
    {
        Slayer::ResourceManager::Initialize();
        Slayer::ResourceManager* rm = Slayer::ResourceManager::Get();
        m_levelLoadFuture = rm->LoadAssetsAsync("pack.slp");
        m_state = AS_Loading;
    }

    void TestbedApplication::InitializeScene()
    {
        Slayer::ForEachComponentType([this]<typename T>() {
            m_store.RegisterComponent<T>();
        });

        Slayer::YamlDeserializer deserializer;
        deserializer.Deserialize(m_store, "../../../Testbed/assets/scene.yml");
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
        m_window.SetEventCallback(SL_EVENT_BIND(TestbedApplication::OnEvent));

        Slayer::Input::Initialize(&m_window);
    }

    void TestbedApplication::InitializeGUI()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)m_window.GetNativeWindow(), true);
        ImGui_ImplOpenGL3_Init("#version 450");
    }

    void TestbedApplication::UpdateGUI()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Set the window stack shoudl be 200x200
        ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
        ImGui::Begin("Loading", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        static auto lastTick = std::chrono::high_resolution_clock::now();
        static std::string text;
        auto now = std::chrono::high_resolution_clock::now();
        if (now - lastTick > std::chrono::milliseconds(250))
        {
            std::stringstream stream;
            stream << std::fixed << std::setprecision(2) << GetDeltaTime() * 1000.0f << " ms" << std::endl;
            stream << std::fixed << std::setprecision(0) << 1.0f / GetDeltaTime() << " FPS" << std::endl;
            stream << "Entities: " << m_store.GetEntityCount() << std::endl;
            text = stream.str();
            lastTick = now;
        }
        ImGui::Text(m_state == AS_Loading ? "Loading..." : text.c_str());
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void TestbedApplication::ShutdownGUI()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}

Slayer::Application* RegisterApplication(int argc, const char** argv)
{
    return new Testbed::TestbedApplication();
}
