#include "cepch.h"
#include "Application.h"

#include "CatEngine/Renderer/RenderAPI.h"
#include "CatEngine/Renderer/Renderer.h"
#include "CatEngine/Renderer/RenderCommand.h"

extern bool g_ApplicationRunning;

#include <glad/glad.h>

float blueVertices[6 * 4] =
{
    -0.75f, -0.25f, 0.0f, 0.0f, 0.0f, 1.0f, // 0 
    -0.50f, -0.25f, 0.0f, 0.0f, 0.0f, 1.0f, // 1
    -0.50f,  0.00f, 0.0f, 0.0f, 0.0f, 1.0f, // 2
    -0.75f,  0.00f, 0.0f, 0.0f, 0.0f, 1.0f, // 3
};

float redVertices[6 * 4] =
{
     0.50f,  0.25f, 0.0f, 1.0f, 0.0f, 0.0f, // 0 
     0.75f,  0.25f, 0.0f, 1.0f, 0.0f, 0.0f, // 1
     0.75f,  0.50f, 0.0f, 1.0f, 0.0f, 0.0f, // 2
     0.50f,  0.50f, 0.0f, 1.0f, 0.0f, 0.0f, // 3
};

uint32_t indices[6]
{
    0, 1, 2,
    2, 3, 0,
};

namespace CatEngine
{
    Application::Application(const ApplicationSpecification& spec)
    {
        s_Instance = this;

        RenderAPI::Set(RenderAPI::API::OpenGL);

        Renderer::Init();

        m_Window.Init(WindowProps(spec.Name, 800, 800));
        m_Window.SetVSync(true);
        m_Window.SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

        m_ImGuiLayer = new ImGuiLayer();
        PushOverlay(m_ImGuiLayer);

        m_BlueVAO = VertexArray::Create();
        m_BlueVBO = VertexBuffer::Create(blueVertices, 6 * 8);
        m_BlueVBO->SetLayout({
            { ShaderDataType::Vec3 },
            { ShaderDataType::Vec3 },
        });
        m_BlueVAO->AddVertexBuffer(m_BlueVBO);
        m_BlueEBO = IndexBuffer::Create(indices, 12);
        m_BlueVAO->SetIndexBuffer(m_BlueEBO);

        m_RedVAO = VertexArray::Create();
        m_RedVBO = VertexBuffer::Create(redVertices, 6 * 8);
        m_RedVBO->SetLayout({
            { ShaderDataType::Vec3 },
            { ShaderDataType::Vec3 },
        });
        m_RedVAO->AddVertexBuffer(m_RedVBO);
        m_RedEBO = IndexBuffer::Create(indices, 12);
        m_RedVAO->SetIndexBuffer(m_RedEBO);

        // Shader
        m_Shader = Shader::Create("resources/shader/vert.glsl", "resources/shader/frag.glsl");
        m_Shader->Bind();

    }

    Application::~Application()
    {
        g_ApplicationRunning = false;

        s_Instance = nullptr;
    }

    void Application::Run()
    {
        m_Running = true;
        while(m_Running)
        {
            // TODO Make a render command class for this e
            RenderCommand::Clear({0.24f, 0.16f, 0.78f, 1.0f});

            Renderer::BeginScene();
            m_Shader->Bind();
            Renderer::Submit(m_BlueVAO);
            Renderer::Submit(m_RedVAO);

            Renderer::EndScene();

            //glDrawElements(GL_TRIANGLES, m_VertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0);

            float time = (float)glfwGetTime();
            float ts = time - m_LastFrameTime;
            m_LastFrameTime = ts;
            
            if (!m_Minimized)
            {
                for (Layer* layer : m_LayerStack)
                    layer->OnUpdate(m_LastFrameTime);
            }

            m_ImGuiLayer->Begin();
            for (Layer* layer : m_LayerStack)
                layer->OnImGuiDraw();
            m_ImGuiLayer->End();

            m_Window.OnUpdate();
        }
    }

    void Application::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
        dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));

        for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
        {
            if (e.Handled)
                break;
            (*it)->OnEvent(e);
        }
    }

    void Application::PushLayer(Layer* layer)
    {
        m_LayerStack.PushLayer(layer);
        layer->OnAttach();
    }

    void Application::PushOverlay(Layer* layer)
    {
        m_LayerStack.PushLayer(layer);
        layer->OnAttach();
    }

    bool Application::OnWindowClose(WindowCloseEvent& e)
    {
        m_Running = false;
        return true;
    }

    bool Application::OnWindowResize(WindowResizeEvent& e)
    {
        if (e.GetWidth() == 0 || e.GetHeight() == 0)
        {
            m_Minimized = true;
            return false;
        }

        m_Minimized = true;

        return false;
    }
}
