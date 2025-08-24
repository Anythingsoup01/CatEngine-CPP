#include "EditorLayer.h"

#include "imgui.h" 
#include <imgui_internal.h>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "ImGuizmo.h"

#include <glad/glad.h>

namespace CatEngine
{        
    class CameraController : public ScriptObject
    {
    public:

        void OnStart() override
        {
        }

        void OnUpdate(Time ts) override
        {
            auto& position = GetComponent<TransformComponent>().Position;
            float speed = 5.0f;

            if (Input::IsKeyPressed(KeyCode::W))
                position.y += speed * ts;
            if (Input::IsKeyPressed(KeyCode::S))
                position.y -= speed * ts;
            if (Input::IsKeyPressed(KeyCode::A))
                position.x -= speed * ts;
            if (Input::IsKeyPressed(KeyCode::D))
                position.x += speed * ts;
        }

    };

    void EditorLayer::OnAttach()
    {
        s_Instance = this;
 		
        m_IconStartRuntime = Texture2D::Create("Resources/Icons/Editor/Start-Runtime.png");
		m_IconPauseRuntime = Texture2D::Create("Resources/Icons/Editor/Pause-Runtime.png");
		m_IconPauseRuntimeSelected = Texture2D::Create("Resources/Icons/Editor/Pause-Runtime-Selected.png");
		m_IconNextFrameRuntime = Texture2D::Create("Resources/Icons/Editor/NextFrame-Runtime.png");
		m_IconStopRuntime = Texture2D::Create("Resources/Icons/Editor/Stop-Runtime.png");

		m_IconStartSimulation = Texture2D::Create("Resources/Icons/Editor/Start-Simulation.png");

        FramebufferSpecification fbSpec;
        fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        m_SceneViewportPanel = SceneViewportPanel(fbSpec);
        m_SceneCameraPanel = SceneViewportPanel(fbSpec);



        m_ActiveScene = CreateRef<Scene>();

        m_CurrentProjectName = "SampleProject";

		m_ProjectAssetsDirectory = std::filesystem::path(m_CurrentProjectName + "/Assets");

        m_MouseInUse = false;


        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
        m_SceneViewportPanel.SetContext(m_ActiveScene);
        m_SceneCameraPanel.SetContext(m_ActiveScene);

        m_EditorCamera = EditorCamera(30.f, 1.778f, 0.1f, 1000.f);
    }

    void EditorLayer::OnDetach()
    {
        s_Instance = nullptr;
        
    }

    void EditorLayer::OnUpdate(Time deltaTime)
    {
        CE_PROFILE_FUNCTION();
        m_DeltaTime = deltaTime;
        switch (m_SceneState)
		{
		    case SceneState::Edit:
            {
                OnUpdateEditor(deltaTime);
                OnUpdateMainCameraPreview(deltaTime);
                break;
            }
            case SceneState::Play:
            {
                OnUpdateRuntime(deltaTime);
                break;
            }
            case SceneState::Simulate:
            {
                OnUpdateSimulation(deltaTime);
                OnUpdateMainCameraPreview(deltaTime);
                break;
            }
            case SceneState::Pause:
            {
                OnUpdateRuntime(deltaTime);
                OnUpdateEditor(deltaTime);
                break;
            }
        }
         
        
    }


    void EditorLayer::OnImGuiDraw()
    {

        CE_PROFILE_FUNCTION();

        static bool dockspaceOpen = true;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

        {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }

        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;


        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("MyDockSpace", &dockspaceOpen, window_flags);
        ImGui::PopStyleVar(3);
        {
            // Submit the DockSpace
            ImGuiIO& io = ImGui::GetIO();
            if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
            {
                ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
            }
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });

            ImGui::Begin("Console");
            {
                Renderer2D::Statistics stats = Renderer2D::GetStats();
                ImGui::Text("Draw Calls: %i", stats.DrawCalls);
                ImGui::Text("Rendered Quads: %i", stats.QuadCount);
                ImGui::Text("Ts: %f", m_DeltaTime);
                ImGui::Text("Rough FPS: %f", 1.0f / m_DeltaTime);
                Renderer2D::ResetStats();

            }
            ImGui::End();

            UI_MenuBar();
            UI_Viewport();
            m_SceneHierarchyPanel.OnImGuiRender();
            m_ContentBrowserPanel.OnImGuiRender();
            UI_Toolbar();


			
			ImGui::PopStyleVar();


        }
        ImGui::End();
    }
    
    void EditorLayer::ImGuizmoDraw(Entity selectedEntity, const glm::mat4& cameraProjection, glm::mat4 cameraView)
    {
        // Entity transform
        auto& tc = selectedEntity.GetComponent<TransformComponent>();
        glm::mat4 transform = tc.GetTransform();

        // Snapping
        bool snap = Input::IsKeyPressed(KeyCode::LeftControl);
        float snapValue = 0.5f; // Snap to 0.5m for translation/scale
        // Snap to 45 degrees for rotation
        if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
            snapValue = 45.0f;

        float snapValues[3] = { snapValue, snapValue, snapValue };

        ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform), nullptr, snap ? snapValues : nullptr);

        if (ImGuizmo::IsUsing())
        {
            glm::vec3 position, rotation, scale;
            Math::DecomposeTransform(transform, position, rotation, scale);

            glm::vec3 deltaRotation = rotation - tc.Rotation;
            tc.Position = position;
            tc.Rotation += deltaRotation;
            tc.Scale = scale;

        }
    }

    void EditorLayer::OnUpdateEditor(Time deltaTime)
    {
        if (m_SceneViewportPanel.IsActive())
        {
            if (m_SceneViewportPanel.FramebufferResized())
            {
                m_EditorCamera.SetViewportSize((float)m_SceneViewportPanel.GetWidth(), (float)m_SceneViewportPanel.GetHeight());
            }

            if (!ImGuizmo::IsUsing() && (!m_BlockMouseEvents))
                m_EditorCamera.OnUpdate(deltaTime);

            m_SceneViewportPanel.BindFramebuffer();
            
            RenderCommand::Clear({ 0.1, 0.1, 0.1, 1.0});
            m_SceneViewportPanel.ClearFramebufferAttachment(1, -1);

            m_ActiveScene->OnUpdateEditor(deltaTime, m_EditorCamera);

            auto [mx, my] = ImGui::GetMousePos();
            const glm::vec2* viewportBounds = m_SceneViewportPanel.GetPanelBounds();
            mx -= viewportBounds[0].x;
            my -= viewportBounds[0].y;
            glm::vec2 viewportSize = viewportBounds[1] - viewportBounds[0];
            my = viewportSize.y - my;

            int mouseX = (int)mx;
            int mouseY = (int)my;

            if ((mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y))
            {
                int pixelData = m_SceneViewportPanel.ReadPixelData(1, mouseX, mouseY);
                m_HoveredEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, m_ActiveScene.get());
            }
            m_SceneViewportPanel.UnbindFramebuffer();
        }

    }

    void EditorLayer::OnUpdateMainCameraPreview(Time deltaTime)
    {
        if (m_SceneCameraPanel.IsActive())
        {
            m_SceneCameraPanel.FramebufferResized();


            m_SceneCameraPanel.BindFramebuffer();
            RenderCommand::Clear({ 0.1, 0.1, 0.1, 1.0});
            m_ActiveScene->OnUpdateMainCameraPreview(deltaTime);

            m_SceneCameraPanel.UnbindFramebuffer();
        }

    }

    void EditorLayer::OnUpdateRuntime(Time deltaTime)
    {
        if (m_SceneCameraPanel.IsActive())
        {
            OnUpdateEditor(deltaTime);
            m_SceneCameraPanel.FramebufferResized();


            m_SceneCameraPanel.BindFramebuffer();
            RenderCommand::Clear({ 0.1, 0.1, 0.1, 1.0});
            m_ActiveScene->OnUpdateRuntime(deltaTime);

            m_SceneCameraPanel.UnbindFramebuffer();
        }
        else if (m_SceneViewportPanel.IsActive())
        {
            m_SceneViewportPanel.BindFramebuffer();

            RenderCommand::Clear({ 0.1, 0.1, 0.1, 1.0});

            m_ActiveScene->OnUpdateRuntime(deltaTime);

            m_SceneViewportPanel.UnbindFramebuffer();

        }
    }

    void EditorLayer::OnUpdateSimulation(Time deltaTime)
    {
        if (m_SceneViewportPanel.IsActive())
        {
            if (m_SceneViewportPanel.FramebufferResized())
            {
                m_EditorCamera.SetViewportSize((float)m_SceneViewportPanel.GetWidth(), (float)m_SceneViewportPanel.GetHeight());
            }

            if (!ImGuizmo::IsUsing() && (!m_BlockMouseEvents))
                m_EditorCamera.OnUpdate(deltaTime);

            m_SceneViewportPanel.BindFramebuffer();
            
            RenderCommand::Clear({ 0.1, 0.1, 0.1, 1.0});
            m_SceneViewportPanel.ClearFramebufferAttachment(1, -1);

            m_ActiveScene->OnUpdateSimulation(deltaTime, m_EditorCamera);

            auto [mx, my] = ImGui::GetMousePos();
            const glm::vec2* viewportBounds = m_SceneViewportPanel.GetPanelBounds();
            mx -= viewportBounds[0].x;
            my -= viewportBounds[0].y;
            glm::vec2 viewportSize = viewportBounds[1] - viewportBounds[0];
            my = viewportSize.y - my;

            int mouseX = (int)mx;
            int mouseY = (int)my;

            if ((mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y))
            {
                int pixelData = m_SceneViewportPanel.ReadPixelData(1, mouseX, mouseY);
                m_HoveredEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, m_ActiveScene.get());
            }
            m_SceneViewportPanel.UnbindFramebuffer();
        }

    }

    void EditorLayer::UI_Viewport()
    {
        if (m_SceneViewportPanel.IsActive())
        {
            ImGui::Begin("Viewport");
            m_SceneViewportPanel.OnImGuiRender();
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_MANAGER_ITEM"))
                {
                    const wchar_t* path = (const wchar_t*)payload->Data;
                    std::filesystem::path filePath = std::filesystem::path("SampleProject/Assets") / path;
    
                    if (filePath.extension().string() == ".catengine")
                    {
                        OpenScene(filePath);
                        m_SceneFilePath = filePath;
                    }
                    else if (filePath.extension().string() == ".png" || filePath.extension().string() == ".jpeg")
                    {
                        std::filesystem::path texturePath = std::filesystem::path("SampleProject/Assets") / path;
                        Ref<Texture2D> texture = Texture2D::Create(texturePath.string());
                        if (texture->IsLoaded())
                        {
                            if (m_HoveredEntity && m_HoveredEntity.HasComponent<SpriteRendererComponent>())
                                m_HoveredEntity.GetComponent<SpriteRendererComponent>().Texture = texture;
                        }
                        else
                        {
                            CE_CLI_WARN("Could not load texture {0}", texturePath.filename().string());
                        }
                    }
                    else
                    {
                    }
                }

                ImGui::EndDragDropTarget();
            }
            UI_Gizmos();
            ImGui::End();
        }

        if (m_SceneCameraPanel.IsActive())
        {
            ImGui::Begin("Main Camera (preview)");
            m_SceneCameraPanel.OnImGuiRender();
            ImGui::End();
        }
    }
	
    void EditorLayer::UI_Gizmos()
	{
		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
		if (selectedEntity && m_GizmoType != -1)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();
            
            const glm::vec2* viewportBounds = m_SceneViewportPanel.GetPanelBounds();

			ImGuizmo::SetRect(viewportBounds[0].x, viewportBounds[0].y, viewportBounds[1].x - viewportBounds[0].x, viewportBounds[1].y - viewportBounds[0].y);


			// Camera

			{
				const glm::mat4& cameraProjection = m_EditorCamera.GetProjection();
				glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();
				ImGuizmoDraw(selectedEntity, cameraProjection, cameraView);
			}


		}
	}
    
    void EditorLayer::UI_Toolbar()
    {
		ImGuiWindowClass tabWindowClass;
		tabWindowClass.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_NoResizeFlagsMask_;
		ImGui::SetNextWindowClass(&tabWindowClass);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 2 });
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, { 0, 0 });
		ImGuiWindowFlags tabStyle = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

		
		
        ImGui::Begin("##toolbar", nullptr, tabStyle);
        {
            float size = ImGui::GetWindowHeight() - 4.f;
            Ref<Texture2D> runtimeIcon = m_SceneState == SceneState::Play ? m_IconStopRuntime : m_IconStartRuntime;
			Ref<Texture2D> pauseIcon;
			if (m_SceneState == SceneState::Edit)
				pauseIcon = m_IsScenePaused ? m_IconPauseRuntimeSelected : m_IconPauseRuntime;
			else if (m_SceneState == SceneState::Play)
				pauseIcon = m_IsScenePaused ? m_IconNextFrameRuntime : m_IconPauseRuntime;
			else if (m_SceneState == SceneState::Simulate)
				pauseIcon = m_IsScenePaused ? m_IconPauseRuntimeSelected : m_IconPauseRuntime; // TODO: Disable the play & pause buttons during simulation

			Ref<Texture2D> simulationIcon = m_SceneState == SceneState::Simulate ? m_IconStopRuntime : m_IconStartSimulation;

            ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f - (size * 0.5f)));
            if (ImGui::ImageButton((ImTextureID)(uint64_t)runtimeIcon->GetRendererID(), ImVec2(size * 1.403f, size), {0, 0}, {1, 1}, 0))
            {
                if (m_SceneState == SceneState::Edit)
                    OnScenePlay();
                else if (m_SceneState == SceneState::Play)
                    OnSceneStop();
            }
			ImGui::SameLine();
			if (ImGui::ImageButton((ImTextureID)(uint64_t)pauseIcon->GetRendererID(), ImVec2(size * 1.403f, size), { 0, 0 }, { 1, 1 }, 0))
			{
				if (m_SceneState == SceneState::Edit)
				{
					m_IsScenePaused = !m_IsScenePaused;
					OnScenePause(m_IsScenePaused);
				}
				else if (m_SceneState == SceneState::Play)
				{
					m_IsScenePaused = !m_IsScenePaused;
					OnScenePause(m_IsScenePaused);
				}
				
			}
			ImGui::SameLine();
			if (ImGui::ImageButton((ImTextureID)(uint64_t)simulationIcon->GetRendererID(), ImVec2(size * 1.403f, size), { 0, 0 }, { 1, 1 }, 0))
			{
				if (m_SceneState == SceneState::Edit)
					OnSceneSimulateStart();
				else if (m_SceneState == SceneState::Simulate)
					OnSceneSimulateStop();
			}

            ImGui::PopStyleVar(2);
        }
        ImGui::End();

    }
	
    void EditorLayer::UI_MenuBar()
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New", "Ctrl+N")) NewScene();

				if (ImGui::MenuItem("Open...", "Ctrl+O")) OpenScene();

				if (ImGui::MenuItem("Save", "Ctrl+S")) SaveScene();

				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) SaveSceneAs();


                // TODO: Be able to close from here!
				//if (ImGui::MenuItem("Exit")) Application::Get().CloseEditor();
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Scene"))
			{
				const char* runtimeText = m_SceneState == SceneState::Edit ? "Start Runtime" : "Stop Runtime";
				if (ImGui::MenuItem(runtimeText, "Ctrl+F5"))  m_SceneState == SceneState::Edit ? OnScenePlay() : OnSceneStop();

				if (ImGui::MenuItem("Simulate Runtime", "Ctrl+F7")) m_SceneState == SceneState::Edit ? OnSceneSimulateStart() : OnSceneSimulateStop();

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
	}

    void EditorLayer::SaveSceneAs()
    {
		CE_PROFILE_FUNCTION();

        if (m_SceneState == SceneState::Edit)
        {
            m_SceneFilePath = FileDialog::SaveFile({{"CatEngine Scene", "catengine"}});
            if (!m_SceneFilePath.empty())
            {
                SceneSerializer serializer(m_ActiveScene);
                serializer.Serialize(m_SceneFilePath.string());
            }
        }
    }

    void EditorLayer::SaveScene()
    {
		CE_PROFILE_FUNCTION();

        if (m_SceneState == SceneState::Edit)
        {
            if (!m_SceneFilePath.empty())
            {
                SceneSerializer serializer(m_ActiveScene);

                serializer.Serialize(m_SceneFilePath.string());
            }
            else SaveSceneAs();
        }
    }

    void EditorLayer::OpenScene()
    {
		CE_PROFILE_FUNCTION();

        m_SceneFilePath = FileDialog::OpenFile({{"CatEngine Scene", "catengine"}});
        if (!m_SceneFilePath.empty())
            OpenScene(m_SceneFilePath);
    }
    void EditorLayer::OpenScene(const std::filesystem::path& filePath)
    {
		CE_PROFILE_FUNCTION();

        if (filePath.extension().string() != ".catengine")
        {
            CE_API_WARN("Could not load {0} - not a scene file", filePath.filename().string());
            return;
        }

        Ref<Scene> newScene = CreateRef<Scene>();
        SceneSerializer serializer(newScene);
        if (serializer.Deserialize(filePath.string()))
        {
            m_EditorScene = newScene;

            m_EditorScene->OnViewportResize(m_SceneViewportPanel.GetWidth(), m_SceneViewportPanel.GetHeight());
            m_ActiveScene = m_EditorScene;
            m_SceneHierarchyPanel.SetContext(m_ActiveScene);
            m_SceneViewportPanel.SetContext(m_ActiveScene);
            m_SceneCameraPanel.SetContext(m_ActiveScene);
        }
    }

    void EditorLayer::NewScene()
    {
		CE_PROFILE_FUNCTION();

        m_SceneFilePath = std::filesystem::path();
        m_EditorScene = CreateRef<Scene>();
        m_EditorScene->OnViewportResize(m_SceneViewportPanel.GetWidth(), m_SceneViewportPanel.GetHeight());
        m_ActiveScene = m_EditorScene;
        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
        m_SceneViewportPanel.SetContext(m_ActiveScene);
        m_SceneCameraPanel.SetContext(m_ActiveScene);

    }

    void EditorLayer::OnScenePlay()
    {
		CE_PROFILE_FUNCTION();

        m_SceneState = SceneState::Play;

        m_EditorScene = Scene::Copy(m_ActiveScene);
        m_ActiveScene->OnRuntimeStart();
        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
        m_SceneViewportPanel.SetContext(m_ActiveScene);
        m_SceneCameraPanel.SetContext(m_ActiveScene);
    }

	void EditorLayer::OnScenePause(bool isPaused)
	{
		if (isPaused)
			m_ActiveScene->OnPauseStart();
		else
			m_ActiveScene->OnPauseStop();
	}

    void EditorLayer::OnSceneStop()
    {
		CE_PROFILE_FUNCTION();

        m_SceneState = SceneState::Edit;

        m_ActiveScene->OnRuntimeStop();
        m_ActiveScene = Scene::Copy(m_EditorScene);
        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
        m_SceneViewportPanel.SetContext(m_ActiveScene);
        m_SceneCameraPanel.SetContext(m_ActiveScene);
    }

    void EditorLayer::OnSceneSimulateStart()
    {
		CE_PROFILE_FUNCTION();

        m_SceneState = SceneState::Simulate;
     
		m_EditorScene = Scene::Copy(m_ActiveScene);
        m_ActiveScene->OnSimulationStart();
        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
        m_SceneViewportPanel.SetContext(m_ActiveScene);
        m_SceneCameraPanel.SetContext(m_ActiveScene);
    }
    void EditorLayer::OnSceneSimulateStop()
    {
		CE_PROFILE_FUNCTION();

        m_SceneState = SceneState::Edit;
        
		m_ActiveScene->OnSimulationStop();
        m_ActiveScene = Scene::Copy(m_EditorScene);
        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
        m_SceneViewportPanel.SetContext(m_ActiveScene);
        m_SceneCameraPanel.SetContext(m_ActiveScene);
    }

    void EditorLayer::DuplicateEntity()
    {
		CE_PROFILE_FUNCTION();

        Entity entity = m_SceneHierarchyPanel.GetSelectedEntity();
        if (entity)
        {
            m_ActiveScene->DuplicateEntity(entity);
        }
    }
    void EditorLayer::DeleteEntity()
    {
		CE_PROFILE_FUNCTION();

        Entity entity = m_SceneHierarchyPanel.GetSelectedEntity();
        if (entity)
        {
            m_ActiveScene->DeleteEntity(entity);
            m_SceneHierarchyPanel.SetSelectedEntity();
        }
    }
    void EditorLayer::CopyEntity()
    {
		CE_PROFILE_FUNCTION();

        Entity entity = m_SceneHierarchyPanel.GetSelectedEntity();
        if (entity)
        {
            m_CopiedEntity = entity;
        }
    }
    void EditorLayer::PasteEntity()
    {
		CE_PROFILE_FUNCTION();

        if (m_CopiedEntity)
        {
            // TODO : Make it to where entity doesn't need to exist
            Entity pastedEntity = m_ActiveScene->PasteEntity(m_CopiedEntity);
            m_SceneHierarchyPanel.SetSelectedEntity(pastedEntity);
        }
    }
    
    void EditorLayer::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        m_EditorCamera.OnEvent(e);
        dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(EditorLayer::OnWindowResize));
        dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(EditorLayer::OnKeyPressed));
        dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
    }

    bool EditorLayer::OnWindowResize(WindowResizeEvent& e)
    {
        Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

        return false;
    }

     bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
    {   

        bool control = Input::IsKeyPressed(KeyCode::LeftControl) || Input::IsKeyPressed(KeyCode::RightControl);
		bool shift = Input::IsKeyPressed(KeyCode::LeftShift) || Input::IsKeyPressed(KeyCode::RightShift);
		// Shortcuts
        switch (e.GetKeyCode())
        {
            case KeyCode::C:
            {
                if (control && shift)
                {
                }
                else if (control)
                    CopyEntity();
                break;
            }
            case KeyCode::D:
            {
                if (control && shift)
                {
                }
                else if (control)
                    DuplicateEntity();
                break;
            }
            case KeyCode::V:
            {
                if (control && shift)
                {
                }
                else if (control)
                    PasteEntity();
                break;
            }
            case KeyCode::S :
            {
                if (control && shift)
                    SaveSceneAs();
                else if (control)
                    SaveScene();
                break;
            }
            case KeyCode::O:
            {
                if (control)
                    OpenScene();
                break;
            }
            case KeyCode::N:
            {
                if (control) 
                    NewScene();
                break;
            }
            case KeyCode::F5:
            {
                if (control)
                    m_SceneState == SceneState::Edit ? OnScenePlay() : OnSceneStop();
                break;
            }
            case KeyCode::F7:
            {
                if (control)
                    m_SceneState == SceneState::Edit ? OnSceneSimulateStart() : OnSceneSimulateStop();
                break;
            }
			case KeyCode::Q:
			{
				if (m_SceneViewportPanel.IsFocused() || m_SceneViewportPanel.IsHovered())
					m_GizmoType = ImGuizmo::OPERATION::BOUNDS;
				break;
			}
            case KeyCode::W:
            {
				if (m_SceneViewportPanel.IsFocused() || m_SceneViewportPanel.IsHovered())
					m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
                break;
            }
            case KeyCode::E:
            {
				if (m_SceneViewportPanel.IsFocused() || m_SceneViewportPanel.IsHovered())
					m_GizmoType = ImGuizmo::OPERATION::ROTATE;
                break;
            }
            case KeyCode::R:
            {
				if (m_SceneViewportPanel.IsFocused() || m_SceneViewportPanel.IsHovered())
					m_GizmoType = ImGuizmo::OPERATION::SCALE;
                break;
            }
            case KeyCode::Delete:
            {
                DeleteEntity();
            }
            default:
                break;
        }

		// Mouse in use checks
	

        return false;
    }

    bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
    {
		bool alt = Input::IsKeyPressed(KeyCode::LeftAlt) || Input::IsKeyPressed(KeyCode::RightAlt);
		if (Input::IsMouseButtonPressed(MouseCode::ButtonLeft))
		{
			if (m_SceneViewportPanel.IsHovered() && !ImGuizmo::IsOver() && !alt)
				m_SceneHierarchyPanel.SetSelectedEntity(m_HoveredEntity);
		}

        return false;
    }



}
