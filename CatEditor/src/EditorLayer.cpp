#include "EditorLayer.h"
#include "imgui.h" 

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
 
        FramebufferSpecification fbSpec;
        fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        m_SceneViewportPanel = SceneViewportPanel(fbSpec);
        m_SceneCameraPanel = SceneViewportPanel(fbSpec);


        m_ActiveScene = CreateRef<Scene>();

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

        if (m_SceneViewportPanel.IsActive())
        {
            if (m_SceneViewportPanel.FramebufferResized())
            {
                m_EditorCamera.SetViewportSize((float)m_SceneViewportPanel.GetWidth(), (float)m_SceneViewportPanel.GetHeight());
            }

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

        if (m_SceneCameraPanel.IsActive())
        {
            m_SceneCameraPanel.FramebufferResized();


            m_SceneCameraPanel.BindFramebuffer();
            RenderCommand::Clear({ 0.1, 0.1, 0.1, 1.0});
            m_ActiveScene->OnUpdateMainCameraPreview(deltaTime);

            m_SceneCameraPanel.UnbindFramebuffer();
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

            UI_Viewport();
            m_SceneHierarchyPanel.OnImGuiRender();
            m_ContentBrowserPanel.OnImGuiRender();


            ImGui::Begin("Console");
            {
                Renderer2D::Statistics stats = Renderer2D::GetStats();
                ImGui::Text("Draw Calls: %i", stats.DrawCalls);
                ImGui::Text("Rendered Quads: %i", stats.QuadCount);
                ImGui::Text("Ts: %f", m_DeltaTime);
                ImGui::Text("Rough FPS: %f", 1.0f / m_DeltaTime);
                Renderer2D::ResetStats();

                ImGui::Checkbox("Use Main Camera", &m_UsingMainCamera);

            }
            ImGui::End();
			
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

    void EditorLayer::OpenScene(const std::filesystem::path& filePath)
    {
        if (filePath.extension().string() != ".catengine")
        {
            CE_CLI_WARN("Could not load {} - Not a scene file!", filePath.filename().string());
            return;
        }

        Ref<Scene> newScene = CreateRef<Scene>();
        SceneSerializer serializer(newScene);
        if (serializer.Deserialize(filePath))
        {
            m_EditorScene = newScene;

            m_EditorScene->OnViewportResize(m_SceneViewportPanel.GetWidth(), m_SceneViewportPanel.GetHeight());
            m_ActiveScene = Scene::Copy(m_EditorScene);
            m_SceneHierarchyPanel.SetContext(m_ActiveScene);
            m_SceneViewportPanel.SetContext(m_ActiveScene);
            m_SceneCameraPanel.SetContext(m_ActiveScene);
        }
    }

    void EditorLayer::SaveScene(const std::filesystem::path& filePath)
    {
        if (filePath.extension().string() != ".catengine")
        {
            CE_CLI_WARN("Could not load {} - Not a scene file!", filePath.filename().string());
            return;
        }
        
        SceneSerializer serializer(m_ActiveScene);
        serializer.Serialize(filePath);

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
        bool isControl = Input::IsKeyPressed(KeyCode::LeftControl) || Input::IsKeyPressed(KeyCode::RightControl);
        bool isShift = Input::IsKeyPressed(KeyCode::LeftShift) || Input::IsKeyPressed(KeyCode::RightShift);

        switch (e.GetKeyCode())
        {
            case KeyCode::O:
            {
                if (isControl)
                {
                    const std::string& filePath = FileDialog::OpenFile({{ "CatEngine", "catengine" }});
                    if (!filePath.empty())
                    {
                        OpenScene(filePath);
                    }
                }
                break;
            }
            case KeyCode::S:
            {
                if (isControl)
                {
                    const std::string& filePath = FileDialog::SaveFile({{"CatEngine", "catengine"}});
                    if(isShift)
                    {

                    }
                    else
                    {
                        SaveScene(filePath);
                    }
                }
                break;
            }
            case KeyCode::Q:
			{
				if (m_SceneViewportPanel.IsFocused())
					m_GizmoType = ImGuizmo::OPERATION::BOUNDS;
				break;
			}
            case KeyCode::W:
            {
				if (m_SceneViewportPanel.IsFocused())
					m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
                break;
            }
            case KeyCode::E:
            {
				if (m_SceneViewportPanel.IsFocused())
					m_GizmoType = ImGuizmo::OPERATION::ROTATE;
                break;
            }
            case KeyCode::R:
            {
				if (isControl && isShift)
                {

                }
				if (m_SceneViewportPanel.IsFocused())
					m_GizmoType = ImGuizmo::OPERATION::SCALE;
                break;
            }
            default: break;
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
