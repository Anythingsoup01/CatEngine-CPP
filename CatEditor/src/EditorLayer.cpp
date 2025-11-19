#include "EditorLayer.h"

#include "imgui.h" 
#include <imgui_internal.h>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "ImGuizmo.h"

#include <glad/glad.h>

#include "CatEngine/Scripting/ScriptEngine.h"

#include "CatEngine/AssetManager/AssetManager.h"
#include "CatEngine/AssetManager/TextureImporter.h"

namespace CatEngine
{
    void EditorLayer::OnAttach()
    {
        s_Instance = this;

        m_IconStartRuntime = TextureImporter::ImportIconTexture({}, "Resources/Icons/Editor/Start-Runtime.png");
        m_IconPauseRuntime = TextureImporter::ImportIconTexture({}, "Resources/Icons/Editor/Pause-Runtime.png");
		m_IconPauseRuntimeSelected = TextureImporter::ImportIconTexture({}, "Resources/Icons/Editor/Pause-Runtime-Selected.png");
		m_IconNextFrameRuntime = TextureImporter::ImportIconTexture({}, "Resources/Icons/Editor/NextFrame-Runtime.png");
		m_IconStopRuntime = TextureImporter::ImportIconTexture({}, "Resources/Icons/Editor/Stop-Runtime.png");

		m_IconStartSimulation = TextureImporter::ImportIconTexture({}, "Resources/Icons/Editor/Start-Simulation.png");


        FramebufferSpecification fbSpec;
        fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        m_SceneViewportPanel = SceneViewportPanel(fbSpec);
        m_SceneCameraPanel = SceneViewportPanel(fbSpec);

        m_EditorScene = CreateRef<Scene>();

        m_MouseInUse = false;

        m_SceneHierarchyPanel.SetContext(m_EditorScene);
        m_SceneViewportPanel.SetContext(m_EditorScene);
        m_SceneCameraPanel.SetContext(m_EditorScene);
        ScriptEngine::SetSceneContext(m_EditorScene);
        m_CurrentScene = m_EditorScene;

        auto commandLineArgs = Application::Get().GetSpecification().CommandlineArgs;
        if (commandLineArgs.Count > 1)
        {
            auto projectFilePath = commandLineArgs[1];
            OpenProject(projectFilePath);
            m_ProjectActive = true;
        }

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

        m_BlockMouseEvents = m_SceneViewportPanel.IsHovered();
        m_BlockKeyboardEvents = m_SceneViewportPanel.IsFocused();

        if (m_ProjectActive)
        {
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
                    OnUpdateEditor(deltaTime);
                    break;
                }
                case SceneState::Simulate:
                {
                    OnUpdateSimulation(deltaTime);
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
        else
        {
            OpenProject();
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
            io.FontGlobalScale = 1.25f;
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
            m_AssetBrowserPanel.OnImGuiRender();
            UI_Toolbar();


			
			ImGui::PopStyleVar();


        }
        ImGui::End();
    }

    void EditorLayer::OnOverlayRender()
    {
        Renderer2D::BeginScene(m_EditorCamera);
        {
            Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();

            if (selectedEntity)
            {
                auto& tc = selectedEntity.GetComponent<TransformComponent>();
                glm::vec3 position = tc.Position;
                glm::mat4 rotation = glm::toMat4(glm::quat(tc.Rotation));
                glm::vec3 scale = tc.Scale;

                glm::mat4 transform = glm::translate(glm::mat4(1.f), position) * rotation * glm::scale(glm::mat4(1.f), scale);
                Renderer2D::DrawRect(transform, glm::vec4(0.85f, 0.1f, 0.85f, 1.0f));
            }
        }
        
        {
            auto view = m_EditorScene->GetAllComponentsWith<TransformComponent, CircleCollider2DComponent>();

            for (auto entity : view)
            {
                auto [tc, cc2D] = view.get<TransformComponent, CircleCollider2DComponent>(entity);
                if (cc2D.ShowColliderBounds)
                {
                    glm::vec3 position = tc.Position + glm::vec3(cc2D.Offset, 0.0001f);
                    glm::vec3 scale = tc.Scale * glm::vec3(cc2D.Radius * 2.f);
                    
                    CircleRendererComponent crc;
                    crc.Color = glm::vec4(0.2f, 0.2f, 0.95f, 1.0f);
                    crc.Thickness = 0.025f;
                    crc.Fade = 0.0001f;

                    glm::mat4 transform = glm::translate(glm::mat4(1.f), position) * glm::scale(glm::mat4(1.f), scale);
                    Renderer2D::DrawCircle(transform, crc, -1);

                }
            }
        }

        {
            auto view = m_EditorScene->GetAllComponentsWith<TransformComponent, BoxCollider2DComponent>();

            for (auto entity : view)
            {
                auto [tc, bc2D] = view.get<TransformComponent, BoxCollider2DComponent>(entity);
                if (bc2D.ShowColliderBounds)
                {
                    glm::vec3 scale = tc.Scale * glm::vec3(bc2D.Size * 2.f, 1.f);

					// Thing to note - You have to translate & rotate Collider2D's after the objecct itself
                    glm::mat4 transform = 
						  glm::translate(glm::mat4(1.f), tc.Position)
                        * glm::rotate(glm::mat4(1.0f), tc.Rotation.z, glm::vec3(0,0,1))
						* glm::translate(glm::mat4(1.0f), glm::vec3(bc2D.Offset, 0.001f))
						* glm::rotate(glm::mat4(1.0f), glm::radians(bc2D.Rotation), glm::vec3(0, 0, 1))
                        * glm::scale(glm::mat4(1.f), scale);

                    Renderer2D::DrawRect(transform, glm::vec4(0.2f, 0.2f, 0.95f, 1.0f));

                }
            }
        }
        Renderer2D::EndScene();
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

            m_EditorCamera.AllowEvents(!ImGuizmo::IsUsing() && m_BlockMouseEvents);

            m_EditorCamera.OnUpdate(deltaTime);

            m_SceneViewportPanel.BindFramebuffer();
            
            RenderCommand::Clear({ 0.1, 0.1, 0.1, 1.0});
            m_SceneViewportPanel.ClearFramebufferAttachment(1, -1);

            m_CurrentScene->OnUpdateEditor(deltaTime, m_EditorCamera);

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
                m_HoveredEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, m_EditorScene.get());
            }

            OnOverlayRender();

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
            m_EditorScene->OnUpdateMainCameraPreview(deltaTime);

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
            m_RuntimeScene->OnUpdateRuntime(deltaTime);

            m_SceneCameraPanel.UnbindFramebuffer();
        }
        else if (m_SceneViewportPanel.IsActive())
        {
            m_SceneViewportPanel.BindFramebuffer();

            RenderCommand::Clear({ 0.1, 0.1, 0.1, 1.0});

            m_RuntimeScene->OnUpdateRuntime(deltaTime);

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

            m_EditorCamera.AllowEvents(!ImGuizmo::IsUsing() && m_BlockMouseEvents);

            m_EditorCamera.OnUpdate(deltaTime);

            m_SceneViewportPanel.BindFramebuffer();
            
            RenderCommand::Clear({ 0.1, 0.1, 0.1, 1.0});
            m_SceneViewportPanel.ClearFramebufferAttachment(1, -1);

            m_SimulationScene->OnUpdateSimulation(deltaTime, m_EditorCamera);

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
                m_HoveredEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, m_EditorScene.get());
            }
            m_SceneViewportPanel.UnbindFramebuffer();

            if (m_SceneCameraPanel.IsActive())
            {
                m_SceneCameraPanel.BindFramebuffer();
            
                RenderCommand::Clear({ 0.1, 0.1, 0.1, 1.0});
                m_SceneCameraPanel.ClearFramebufferAttachment(1, -1);
                m_SimulationScene->OnUpdateMainCameraPreview(deltaTime);
                m_SceneCameraPanel.UnbindFramebuffer();

            }
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
                    /*
                    const wchar_t* path = (const wchar_t*)payload->Data;
                    std::filesystem::path filePath = Project::GetAssetFileSystemPath(path);
    
                    if (filePath.extension().string() == ".catscene")
                    {
                        OpenScene(filePath);
                        SaveProject();
                    }
                    else if (filePath.extension().string() == ".png" || filePath.extension().string() == ".jpeg")
                    {
                        std::filesystem::path texturePath = Project::GetAssetFileSystemPath(path);
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
                    */

                    AssetHandle handle = *(AssetHandle*)payload->Data;
                    const AssetType type = AssetManager::GetAssetType(handle);

                    switch (type)
                    {
                        case AssetType::Scene:
                        {
                            OpenScene(handle);
                            break;
                        }
                        case AssetType::Texture2D:
                        {
                            if (m_HoveredEntity && m_HoveredEntity.HasComponent<SpriteRendererComponent>())
                                m_HoveredEntity.GetComponent<SpriteRendererComponent>().Texture = handle;
                            break;
                        }
                        default: break;
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
            if (ImGui::ImageButton("##PLAY", (ImTextureID)(uint64_t)runtimeIcon->GetRendererID(), ImVec2(size * 1.403f, size), {0, 0}, {1, 1}))
            {
                if (m_SceneState == SceneState::Edit)
                    OnScenePlay();
                else if (m_SceneState == SceneState::Play)
                    OnSceneStop();
            }
			ImGui::SameLine();
			if (ImGui::ImageButton("##PAUSE", (ImTextureID)(uint64_t)pauseIcon->GetRendererID(), ImVec2(size * 1.403f, size), { 0, 0 }, { 1, 1 }))
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
			if (ImGui::ImageButton("##SIMULATE", (ImTextureID)(uint64_t)simulationIcon->GetRendererID(), ImVec2(size * 1.403f, size), { 0, 0 }, { 1, 1 }))
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

				//if (ImGui::MenuItem("Open...", "Ctrl+O")) OpenScene();

				if (ImGui::MenuItem("Save", "Ctrl+S")) SaveScene();

				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) SaveSceneAs();


                // TODO: Be able to close from here!
				// if (ImGui::MenuItem("Exit")) Application::Get().CloseEditor();
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
            m_EditorScenePath = FileDialog::SaveFile({{"CatEngine Scene", "catscene"}});
            if (!m_EditorScenePath.empty())
                SaveScene();

            std::filesystem::path relativePath = std::filesystem::relative(m_EditorScenePath, Project::GetAssetDirectory());
            AssetHandle handle = Project::GetActive()->GetEditorAssetManager()->ImportAsset(relativePath);
            m_CurrentSceneHandle = handle;
            m_AssetBrowserPanel.ResetAssetDirectory();
        }
    }

    void EditorLayer::SaveScene()
    {
		CE_PROFILE_FUNCTION();

        if (m_SceneState == SceneState::Edit)
        {
            if (!m_EditorScenePath.empty())
            {
                SceneSerializer serializer(m_EditorScene);
                serializer.Serialize(m_EditorScenePath.string());
                if (AssetManager::IsAssetHandleValid(m_CurrentSceneHandle))
                {
                    m_EditorScenePath = Project::GetAssetDirectory() / Project::GetActive()->GetEditorAssetManager()->GetFilePath(m_CurrentSceneHandle);
                }
                CE_API_WARN("Attempting to save at {}", m_EditorScenePath.c_str());
            }
            else SaveSceneAs();

        }
    }

    void EditorLayer::OpenScene(AssetHandle handle)
    {
		CE_PROFILE_FUNCTION();

        Ref<Scene> readOnlyScene = AssetManager::GetAsset<Scene>(handle);
        Ref<Scene> newScene = CreateRef<Scene>();
        readOnlyScene->CopyTo(newScene);


        m_EditorScene = newScene;
        

        m_EditorScene->OnViewportResize(m_SceneViewportPanel.GetWidth(), m_SceneViewportPanel.GetHeight());
        m_SceneHierarchyPanel.SetContext(m_EditorScene);
        m_SceneViewportPanel.SetContext(m_EditorScene);
        m_SceneCameraPanel.SetContext(m_EditorScene);
        ScriptEngine::SetSceneContext(m_EditorScene);

        m_CurrentScene = m_EditorScene;

        std::filesystem::path savePath = Project::GetAssetDirectory() / Project::GetActive()->GetEditorAssetManager()->GetFilePath(handle);

        m_EditorScenePath = savePath;
        m_CurrentSceneHandle = handle;
    }

    void EditorLayer::NewScene()
    {
		CE_PROFILE_FUNCTION();

        m_CurrentSceneHandle = 0;
        m_EditorScenePath = std::filesystem::path();

        m_EditorScene = CreateRef<Scene>();
        m_EditorScene->OnViewportResize(m_SceneViewportPanel.GetWidth(), m_SceneViewportPanel.GetHeight());
        m_SceneHierarchyPanel.SetContext(m_EditorScene);
        m_SceneViewportPanel.SetContext(m_EditorScene);
        m_SceneCameraPanel.SetContext(m_EditorScene);
        ScriptEngine::SetSceneContext(m_EditorScene);

        m_CurrentScene = m_EditorScene;
    }

    void EditorLayer::SaveProject()
    {
        Project::SetCurrentScene(m_CurrentSceneHandle);
        Project::Save(m_CurrentProjectPath);
    }

    void EditorLayer::OpenProject()
    {
        std::filesystem::path filePath = FileDialog::OpenFile({{"CatEngine Project", "catproj"}});
        if (!filePath.empty())
            OpenProject(filePath);

    }

    void EditorLayer::OpenProject(const std::filesystem::path& filePath)
    {
        if (filePath.extension().string() != ".catproj")
        {
            CE_CLI_WARN("Could not load {0} - Not a project file!", filePath.filename().string());
            return;
        }
        if(Project::Load(filePath))
        {
            m_CurrentProjectPath = filePath;
            m_ProjectActive = true;
            auto startScenePath = Project::GetConfig().StartScene;
            if (AssetManager::IsAssetHandleValid(startScenePath))
                OpenScene(startScenePath);
            ScriptEngine::InitializeFileSystems();
            m_ContentBrowserPanel.ResetAssetDirectory();
            m_AssetBrowserPanel.ResetAssetDirectory();
        }
    }

    void EditorLayer::NewProject()
    {
        Project::New();
    }




    void EditorLayer::OnScenePlay()
    {
		CE_PROFILE_FUNCTION();

        m_SceneState = SceneState::Play;

        m_RuntimeScene = CreateRef<Scene>();

        m_EditorScene->CopyTo(m_RuntimeScene);

        m_SceneHierarchyPanel.SetContext(m_RuntimeScene);
        m_SceneViewportPanel.SetContext(m_RuntimeScene);
        m_SceneCameraPanel.SetContext(m_RuntimeScene);
        ScriptEngine::SetSceneContext(m_RuntimeScene);
        m_RuntimeScene->OnRuntimeStart();
        m_CurrentScene = m_RuntimeScene;

        if (m_IsScenePaused)
            m_CurrentScene->OnPauseStart();

    }

	void EditorLayer::OnScenePause(bool isPaused)
	{

        if (m_SceneState == SceneState::Edit)
            return;

		if (isPaused)
			m_CurrentScene->OnPauseStart();
		else
			m_CurrentScene->OnPauseStop();
	}

    void EditorLayer::OnSceneStop()
    {
		CE_PROFILE_FUNCTION();

        m_SceneState = SceneState::Edit;
        m_RuntimeScene->OnRuntimeStop();

        m_RuntimeScene = nullptr;

        m_SceneHierarchyPanel.SetContext(m_EditorScene);
        m_SceneViewportPanel.SetContext(m_EditorScene);
        m_SceneCameraPanel.SetContext(m_EditorScene);
        ScriptEngine::SetSceneContext(m_EditorScene);
        m_CurrentScene = m_EditorScene;
    }

    void EditorLayer::OnSceneSimulateStart()
    {
		CE_PROFILE_FUNCTION();

        m_SceneState = SceneState::Simulate;
     
        m_SimulationScene = CreateRef<Scene>();

		m_EditorScene->CopyTo(m_SimulationScene);
        m_SceneHierarchyPanel.SetContext(m_SimulationScene);
        m_SceneViewportPanel.SetContext(m_SimulationScene);
        m_SceneCameraPanel.SetContext(m_SimulationScene);
        ScriptEngine::SetSceneContext(m_SimulationScene);
        m_SimulationScene->OnSimulationStart();
        m_CurrentScene = m_SimulationScene;
    }

    void EditorLayer::OnSceneSimulateStop()
    {
		CE_PROFILE_FUNCTION();

        m_SceneState = SceneState::Edit;
        m_SimulationScene->OnSimulationStop();


        m_SimulationScene = nullptr;

        m_SceneHierarchyPanel.SetContext(m_EditorScene);
        m_SceneViewportPanel.SetContext(m_EditorScene);
        m_SceneCameraPanel.SetContext(m_EditorScene);
        ScriptEngine::SetSceneContext(m_EditorScene);
        m_CurrentScene = m_EditorScene;
    }

    void EditorLayer::DuplicateEntity()
    {
		CE_PROFILE_FUNCTION();

        Entity entity = m_SceneHierarchyPanel.GetSelectedEntity();
        if (entity)
        {
            m_CurrentScene->DuplicateEntity(entity);
        }
    }
    void EditorLayer::DeleteEntity()
    {
		CE_PROFILE_FUNCTION();

        Entity entity = m_SceneHierarchyPanel.GetSelectedEntity();
        if (entity)
        {
            m_CurrentScene->DeleteEntity(entity);
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
            Entity pastedEntity = m_CurrentScene->PasteEntity(m_CopiedEntity);
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
                {
                    SaveSceneAs();
                    SaveProject();
                }
                else if (control)
                {
                    SaveScene();
                    SaveProject();
                }
                break;
            }
            case KeyCode::O:
            {
                if (control)
                {
                    //OpenScene();
                    SaveProject();
                }
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
				if (m_BlockKeyboardEvents) // if false, block events
					m_GizmoType = ImGuizmo::OPERATION::BOUNDS;
				break;
			}
            case KeyCode::W:
            {
				if (m_BlockKeyboardEvents)
					m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
                break;
            }
            case KeyCode::E:
            {
				if (m_BlockKeyboardEvents)
					m_GizmoType = ImGuizmo::OPERATION::ROTATE;
                break;
            }
            case KeyCode::R:
            {
				if (m_BlockKeyboardEvents)
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
