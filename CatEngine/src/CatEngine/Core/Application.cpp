#include "cepch.h"
#include "Application.h"

#include "CatEngine/Renderer/RenderAPI.h"

extern bool g_ApplicationRunning;

#include <glad/glad.h>

float vertices[6 * 4] =
{
    -0.5f, -0.5f, 0.0f, 1.0f, 0.25f, 0.15f, // 0 
     0.5f, -0.5f, 0.0f, 0.15f, 1.0f, 0.25f, // 1
     0.5f,  0.5f, 0.0f, 0.25f, 0.15f, 1.0f, // 2
    -0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f,   // 3 
};

uint32_t indices[6]
{
    0, 1, 2,
    2, 3, 0
};

namespace CatEngine
{
    Application::Application(const ApplicationSpecification& spec)
    {
        s_Instance = this;

        RenderAPI::Set(RenderAPI::API::OpenGL);

        m_Window.Init(WindowProps(spec.Name, 800, 800));
        m_Window.SetVSync(true);
        m_Window.SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

        m_ImGuiLayer = new ImGuiLayer();
        PushOverlay(m_ImGuiLayer);

        // Vertex Array
        m_VertexArray = VertexArray::Create();
        // Vertex Buffer
        m_VertexBuffer = VertexBuffer::Create(vertices, 6 * 4);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, nullptr);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (const void*)(sizeof(float) * 3));
        // Index Buffer
        m_IndexBuffer = IndexBuffer::Create(indices, 6);
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
            // TODO Make a render command class for this
            glClear(GL_COLOR_BUFFER_BIT);

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

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
