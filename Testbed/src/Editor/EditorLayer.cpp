#include "Editor/EditorLayer.h"

#include "Core/Application.h"
#include "Core/Events.h"
#include "Input/Input.h"

#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"

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
        SL_EVENT();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Set the window stack shoudl be 200x200
        ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
        ImGui::Begin("Loading", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        static auto lastTick = std::chrono::high_resolution_clock::now();
        static std::string text;
        static auto totalFrames = 0;
        static auto totalSeconds = 0.0f;
        totalFrames++;
        totalSeconds += m_deltaTime;
        auto now = std::chrono::high_resolution_clock::now();
        if (now - lastTick > std::chrono::milliseconds(250))
        {
            float avg = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTick).count() / (float)totalFrames;

            std::stringstream stream;
            stream << std::fixed << std::setprecision(2) << avg << " ms" << std::endl;
            stream << std::fixed << std::setprecision(0) << 1000.0f / avg << " FPS" << std::endl;
            text = stream.str();
            lastTick = now;
            totalFrames = 0;
            totalSeconds = 0.0f;
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
