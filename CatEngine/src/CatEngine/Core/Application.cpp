#include "cepch.h"
#include "Application.h"

extern bool g_ApplicationRunning;

#include <glad/glad.h>

float vertices[3 * 4] =
{
    -0.5f, -0.5f, 0.0f, // 0 
     0.5f, -0.5f, 0.0f, // 1
     0.5f,  0.5f, 0.0f, // 2
    -0.5f,  0.5f, 0.0f  // 3 
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

        m_Window.Init(WindowProps(spec.Name, 800, 800));
        m_Window.SetVSync(true);
        m_Window.SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

        m_ImGuiLayer = new ImGuiLayer();
        PushOverlay(m_ImGuiLayer);

        // Vertex Array
        glGenVertexArrays(1, &m_VertexArray);
        glBindVertexArray(m_VertexArray);

        // Vertex Buffer
        glGenBuffers(1, &m_VertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * 4, vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, nullptr);
        // Index Buffer
        glGenBuffers(1, &m_IndexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 6, indices, GL_STATIC_DRAW);
        // Shader

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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

            glBindVertexArray(m_VertexArray);

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
