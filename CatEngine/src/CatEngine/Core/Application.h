#pragma once

#include "CatEngine/Event/Event.h"
#include "CatEngine/Event/ApplicationEvent.h"

#include "Window.h"

#include "CatEngine/ImGui/ImGuiLayer.h"
#include "CatEngine/Layer/LayerStack.h"

#include "CatEngine/Renderer/Shader.h"
#include "CatEngine/Renderer/Buffer.h"
#include "CatEngine/Renderer/VertexArray.h"

namespace CatEngine
{
	struct ApplicationCommandLineArgs
	{
		int Count = 0;
		char** Args = nullptr;

		const char* operator[](int index) const
		{
			CE_API_ASSERT(index < Count, "");
			return Args[index];
		}
	};

	struct ApplicationSpecification
	{
		std::string Name = "CatEngine";
		std::string WorkingDirectory;
		ApplicationCommandLineArgs CommandlineArgs;
	};
    class Application
    {
    public:
        Application(const ApplicationSpecification& spec);
        ~Application();

        void Run();
        void OnEvent(Event& e);

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* overlay);

        static Application& Get() { return *s_Instance; }
        Window& GetWindow() { return m_Window; }

    private:
        bool OnWindowClose(WindowCloseEvent& e);
        bool OnWindowResize(WindowResizeEvent& e);

    private:

        Scope<Shader> m_Shader;
        
        Ref<VertexBuffer> m_BlueVBO;
        Ref<IndexBuffer> m_BlueEBO;
        Ref<VertexArray> m_BlueVAO;

        Ref<VertexBuffer> m_RedVBO;
        Ref<IndexBuffer> m_RedEBO;
        Ref<VertexArray> m_RedVAO;

        bool m_Running = false;
        bool m_Minimized = false;

        Window m_Window;
        float m_LastFrameTime = 0.0f;

        ImGuiLayer* m_ImGuiLayer;
        LayerStack m_LayerStack;

        static inline Application* s_Instance = nullptr;
    };

    Application* CreateApplication(const ApplicationCommandLineArgs& args);
}
