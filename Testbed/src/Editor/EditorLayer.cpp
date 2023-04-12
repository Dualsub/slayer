#include "Editor/EditorLayer.h"

#include "Core/Application.h"
#include "Core/Events.h"
#include "Input/Input.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

namespace Slayer {

    EditorLayer::EditorLayer()
    {
    }

    EditorLayer::~EditorLayer()
    {
    }

    void EditorLayer::OnAttach()
    {
        InitializeGUI();
    }

    void EditorLayer::OnDetach()
    {
        ShutdownGUI();
    }

    void EditorLayer::OnUpdate(Timespan ts)
    {
        m_deltaTime = ts;
    }

    void EditorLayer::OnRender()
    {
        UpdateGUI();
    }

    void EditorLayer::OnEvent(Event& e)
    {
        SL_EVENT_DISPATCH(Slayer::KeyPressEvent, Slayer::EditorLayer::OnKeyPress);
    }

    bool EditorLayer::OnKeyPress(KeyPressEvent& e)
    {
        switch (e.key)
        {
        case SlayerKey::KEY_ESCAPE:
            Application::Get()->Stop();
            break;
        case Slayer::KEY_R:
            if (Input::IsKeyPressed(SlayerKey::KEY_LEFT_CONTROL))
            {

            }
            break;
        default:
            break;
        }

        return true;
    }

    void EditorLayer::InitializeGUI()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        ImGui::StyleColorsDark();
        void* nativeWindow = Application::Get()->GetWindow().GetNativeWindow();
        ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)nativeWindow, true);
        ImGui_ImplOpenGL3_Init("#version 450");
    }

    void EditorLayer::UpdateGUI()
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
            stream << std::fixed << std::setprecision(2) << m_deltaTime * 1000.0f << " ms" << std::endl;
            stream << std::fixed << std::setprecision(0) << 1.0f / m_deltaTime << " FPS" << std::endl;
            text = stream.str();
            lastTick = now;
        }
        ImGui::Text(text.c_str());
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void EditorLayer::ShutdownGUI()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}
