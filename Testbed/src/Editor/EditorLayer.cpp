#include "Editor/EditorLayer.h"

#include "Core/Application.h"
#include "Core/Events.h"
#include "Input/Input.h"
#include "Scene/World.h"
#include "Editor/EditorActions.h"
#include "Serialization/YamlSerializer.h"

#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"

namespace Slayer::Editor {

    namespace Panels {

        void RenderMenuBar(auto&& saveScene = []() {}, auto&& loadScene = []() {})
        {
            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("New", "Ctrl+N"))
                    {

                    }

                    if (ImGui::MenuItem("Open", "Ctrl+O"))
                    {
                        loadScene();
                    }

                    if (ImGui::MenuItem("Save", "Ctrl+S"))
                    {
                        saveScene();
                    }

                    if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
                    {

                    }

                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
        }

        void RenderScenePanel(Entity& selectedEntity)
        {
            const auto treeNodeFlags = ImGuiTreeNodeFlags_Leaf;
            auto& store = World::GetWorldStore();
            auto entities = store.GetAllEntities();
            Entity enitityToDelete = SL_INVALID_ENTITY;

            // List of entities in the scene
            if (!ImGui::Begin("Scene Tree"))
            {
                ImGui::End();
                return;
            }

            // Create entity button
            if (ImGui::Button("Create Entity"))
            {
                selectedEntity = store.CreateEntity();
            }

            for (auto& entity : entities)
            {
                ImGui::PushID(entity);

                std::string name = store.HasComponent<TagComponent>(entity) ? store.GetComponent<TagComponent>(entity)->tag : "Entity " + std::to_string(entity);

                if (ImGui::TreeNodeEx(name.c_str(), treeNodeFlags))
                {
                    if (ImGui::IsItemClicked())
                    {
                        selectedEntity = entity;
                    }

                    if (ImGui::BeginPopupContextItem("Entity Context Menu"))
                    {
                        if (ImGui::MenuItem("Delete Entity"))
                        {
                            enitityToDelete = entity;
                            if (entity == selectedEntity)
                                selectedEntity = SL_INVALID_ENTITY;
                        }

                        ImGui::EndPopup();
                    }

                    ImGui::TreePop();
                }

                ImGui::PopID();
            }

            ImGui::End();

            if (enitityToDelete != SL_INVALID_ENTITY)
            {
                store.DestroyEntity(enitityToDelete);
            }
        }

        void RenderInspectorPanel(PropertySerializer& serializer, Entity& selectedEntity)
        {
            auto& store = World::GetWorldStore();
            const auto treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen;

            // Scene tree view panel, with ImGui tree
            ImGui::Begin("Entity Inspector");

            Entity entity = selectedEntity;

            if (entity == SL_INVALID_ENTITY)
            {
                ImGui::Text("No entity selected");
                ImGui::End();
                return;
            }

            if (ImGui::BeginPopupContextItem("Add Component"))
            {
                ForEachComponentType([&store, &selectedEntity]<typename T>() {
                    if (store.HasComponent<T>(selectedEntity))
                        return;

                    std::string typeName = GetSanitizedTypeName<T>();
                    if (ImGui::MenuItem(typeName.c_str()))
                    {
                        store.AddComponent(selectedEntity, T());
                    }
                });

                ImGui::EndPopup();
            }

            // Add component button
            if (ImGui::Button("Add Component"))
            {
                ImGui::OpenPopup("Add Component");
            }

            ForEachComponentType([entity, &store, &serializer]<typename T>()
            {
                if (!store.HasComponent<T>(entity))
                    return;

                std::string typeName = GetSanitizedTypeName<T>();
                ImGui::PushID(typeName.c_str());

                T* component = store.GetComponent<T>(entity);

                if (ImGui::TreeNodeEx(typeName.c_str(), treeNodeFlags))
                {
                    if (ImGui::BeginPopupContextItem("Component Context Menu"))
                    {
                        if (ImGui::MenuItem("Remove Component"))
                        {
                            store.RemoveComponent<T>(entity);
                        }

                        ImGui::EndPopup();
                    }

                    component->Transfer(serializer);

                    ImGui::TreePop();
                }
                ImGui::PopID();
            });

            ImGui::End();

            selectedEntity = entity;
        }
    }


    EditorLayer::EditorLayer()
    {
    }

    EditorLayer::~EditorLayer()
    {
    }

    void EditorLayer::OnAttach()
    {
        auto& store = m_loadSceneStore;
        ForEachComponentType([&store]<typename T>() {
            store.RegisterComponent<T>();
        });

        InitializeGUI();
        LoadSettings();
        // File exists
        if (std::filesystem::exists(m_settings.lastScenePath))
        {
            LoadScene(m_settings.lastScenePath);
        }
    }

    void EditorLayer::OnDetach()
    {
        ShutdownGUI();
    }

    void EditorLayer::OnUpdate(Timespan ts)
    {
        m_deltaTime = ts;

        if (!m_loadingScene)
            return;

        if (m_loadSceneFuture.valid() && m_loadSceneFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {
            m_loadSceneFuture.get();
            auto* world = World::Get();
            world->SetStore(m_loadSceneStore);
            m_loadSceneStore = ComponentStore();
            auto& store = m_loadSceneStore;
            ForEachComponentType([&store]<typename T>() {
                store.RegisterComponent<T>();
            });
            m_loadingScene = false;
        }
    }

    void EditorLayer::OnRender()
    {
        UpdateGUI();
    }

    void EditorLayer::OnEvent(Event& e)
    {
        SL_EVENT_DISPATCH(Slayer::KeyPressEvent, Slayer::Editor::EditorLayer::OnKeyPress);
        SL_EVENT_DISPATCH(Slayer::MouseMoveEvent, Slayer::Editor::EditorLayer::OnMouseMove);
    }

    bool EditorLayer::OnKeyPress(KeyPressEvent& e)
    {
        switch (e.key)
        {
        case SlayerKey::KEY_ESCAPE:
            Application::Get()->Stop();
            break;
        case Slayer::KEY_TAB:
            if (Input::IsKeyPressed(SlayerKey::KEY_LEFT_SHIFT))
            {
                ToggleEditMode();
            }
            break;
        case Slayer::KEY_P:

            break;
        case Slayer::KEY_S:
            if (Input::IsKeyPressed(SlayerKey::KEY_LEFT_CONTROL) && m_editMode)
            {
            }
            break;
        case Slayer::KEY_O:
            if (Input::IsKeyPressed(SlayerKey::KEY_LEFT_CONTROL) && m_editMode)
            {
            }
            break;
        case Slayer::KEY_N:
            if (Input::IsKeyPressed(SlayerKey::KEY_LEFT_CONTROL))
            {
                NewScene();
            }
            break;
        default:
            break;
        }

        return m_editMode;
    }

    bool EditorLayer::OnMouseMove(MouseMoveEvent& e)
    {
        return m_editMode; // Capture mouse movement only when in edit mode
    }

    void EditorLayer::InitializeGUI()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
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
        totalFrames++;
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
        }
        ImGui::Text(text.c_str());
        ImGui::End();

        static bool open = true;
        const auto windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse;

        if (m_editMode)
        {
            if (!ImGui::Begin("Editor", &open, windowFlags))
            {
                ImGui::End();
                return;
            }

            ImGui::DockSpace(ImGui::GetID("EditorDockSpace"));

            auto saveScene = [this]() {
                SaveScene(World::GetWorldStore(), "C:/dev/repos/Slayer/Testbed/assets/scene.yml");
                };

            auto loadScene = [this]() {
                LoadScene("C:/dev/repos/Slayer/Testbed/assets/scene.yml");
                };

            Panels::RenderMenuBar(saveScene, loadScene);
            Panels::RenderScenePanel(m_selectedEntity);
            Panels::RenderInspectorPanel(m_propertySerializer, m_selectedEntity);
            ImGui::ShowDemoWindow();

            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void EditorLayer::ShutdownGUI()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void EditorLayer::ToggleEditMode()
    {
        m_editMode = !m_editMode;
        auto& window = Application::Get()->GetWindow();
        window.SetCursorEnabled(m_editMode);
    }

    void EditorLayer::SaveScene(ComponentStore& store, const std::string& path)
    {
        Editor::Actions::SaveScene(store, path);
    }

    void EditorLayer::LoadScene(const std::string& path)
    {
        m_loadingScene = true;
        m_loadSceneFuture = Editor::Actions::LoadScene(m_loadSceneStore, path);
        m_settings.lastScenePath = path;
        SaveSettings();
    }

    void EditorLayer::NewScene()
    {
        auto* world = World::Get();
        auto newStore = ComponentStore();
        ForEachComponentType([&newStore]<typename T>() {
            newStore.RegisterComponent<T>();
        });
        world->SetStore(newStore);
    }


    void EditorLayer::LoadSettings()
    {
        constexpr std::string settingsPath = "settings.yml";
        if (!std::filesystem::exists(settingsPath))
            return;

        YamlDeserializer deserializer;
        deserializer.Deserialize(m_settings, settingsPath);
    }

    void EditorLayer::SaveSettings()
    {
        constexpr std::string settingsPath = "settings.yml";
        YamlSerializer serializer;
        serializer.Serialize(m_settings, settingsPath);
    }

}
