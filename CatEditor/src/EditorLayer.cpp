#include "EditorLayer.h"
#include "imgui.h" 

#include "glm/gtc/matrix_transform.hpp"
#include "glm/glm.hpp"

#include <glad/glad.h>

namespace CatEngine
{
    void EditorLayer::OnAttach()
    {
        s_Instance = this;
        
        FramebufferSpecification fbSpec;
        fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        m_Framebuffer = Framebuffer::Create(fbSpec);

        m_Texture = Texture2D::Create("resources/Textures/car.png");

        m_ActiveScene = CreateRef<Scene>();

        m_Car = m_ActiveScene->CreateEntity("Steve");
        m_Car.AddComponent<SpriteRendererComponent>(glm::vec4(1.0f), m_Texture);

        m_CarViewer = m_ActiveScene->CreateEntity("CarViewer");
        m_CarViewer.AddComponent<CameraComponent>();

        m_CarViewer.GetComponent<TransformComponent>().Position = {0, 0, -0.5f};

    }

    void EditorLayer::OnDetach()
    {
        s_Instance = nullptr;
        
    }

    void EditorLayer::OnUpdate(Time deltaTime)
    {
        CE_PROFILE_FUNCTION();
        m_DeltaTime = deltaTime;
		if (FramebufferSpecification spec = m_Framebuffer->GetSpecification();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized framebuffer is invalid
			(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		{
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
        }
        m_EditorCamera.OnUpdate(deltaTime);

        m_Framebuffer->Bind();

        RenderCommand::Clear({ 0.1, 0.1, 0.1, 1.0});
        m_Framebuffer->ClearAttachment(1, -1);

        m_ActiveScene->OnUpdateEditor(deltaTime, m_EditorCamera, m_UsingMainCamera);


        m_Framebuffer->Unbind();

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

            ImGui::Begin("Console");
            {
                if (m_Car)
                {
                    ImGui::Text("Behold Car!");
                    auto& carPos = m_Car.GetComponent<TransformComponent>().Position;
                    auto& carRot = m_Car.GetComponent<TransformComponent>().Rotation;
                    auto& carSize = m_Car.GetComponent<TransformComponent>().Scale;

                    ImGui::DragFloat3("Position", &carPos.x, 0.25f);
                    ImGui::DragFloat3("Rotation", &carRot.x, 0.25f);
                    ImGui::DragFloat3("Size", &carSize.x, 0.25f);

                }

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

    void EditorLayer::UI_Viewport()
    {
        CE_PROFILE_FUNCTION();
        ImGui::Begin("Scene View");
        auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
        auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
        auto viewportOffset = ImGui::GetWindowPos();


        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        m_ViewportSize = {viewportPanelSize.x, viewportPanelSize.y};

        uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID(0);
        ImGui::Image((void*)(uint64_t)textureID, viewportPanelSize, {0, 1}, {1, 0});

        ImGui::End();
    }

    void EditorLayer::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        m_EditorCamera.OnEvent(e);
        dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(EditorLayer::OnWindowResize));
    }

    bool EditorLayer::OnWindowResize(WindowResizeEvent& e)
    {
        Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

        return false;
    }


}
