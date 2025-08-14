#include "cepch.h"
#include "Application.h"

#include "CatEngine/Renderer/RenderAPI.h"
#include "CatEngine/Renderer/Renderer.h"

#include "Input.h"
#include "TimeStep.h"

extern bool g_ApplicationRunning;



namespace CatEngine
{
    Application::Application(const ApplicationSpecification& spec)
    {
        CE_PROFILE_FUNCTION();
        s_Instance = this;

        RenderAPI::Set(RenderAPI::API::OpenGL);


        m_Window.Init(WindowProps(spec.Name, 1280, 720));
        m_Window.SetVSync(false);
        m_Window.SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

        Renderer::Init();

        m_ImGuiLayer = new ImGuiLayer();
        PushOverlay(m_ImGuiLayer);

    }

    Application::~Application()
    {
        CE_PROFILE_FUNCTION();

        Renderer::Shutdown();

        s_Instance = nullptr;

        g_ApplicationRunning = false;
    }

    void Application::Run()
    {
        CE_PROFILE_FUNCTION();
        m_Running = true;
        while(m_Running)
        {
            float time = Time::GetTime();
            Time deltaTime = time - m_LastFrameTime;
            m_LastFrameTime = time;


            
            if (!m_Minimized)
            {
                for (Layer* layer : m_LayerStack)
                    layer->OnUpdate(deltaTime);
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
        CE_PROFILE_FUNCTION();
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
        CE_PROFILE_FUNCTION();
        m_LayerStack.PushLayer(layer);
        layer->OnAttach();
    }

    void Application::PushOverlay(Layer* layer)
    {
        CE_PROFILE_FUNCTION();
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
        CE_PROFILE_FUNCTION();
        if (e.GetWidth() == 0 || e.GetHeight() == 0)
        {
            m_Minimized = true;
            return false;
        }

        m_Minimized = false;

        return false;
    }
}
