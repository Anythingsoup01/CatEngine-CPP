#pragma once

#include "cepch.h"

#include "CatEngine/Event/Event.h"

#include <GLFW/glfw3.h>

namespace CatEngine 
{
	struct WindowProps 
    {
		std::string Title;
		uint32_t Width;
		uint32_t Height;

		WindowProps(const std::string title = "CatEngine", uint32_t width = 1600, uint32_t height = 900)
			:Title(title), Width(width), Height(height) {}
	};

	class Window {
	public:
		using EventCallbackFn = std::function<void(Event&)>;
        void Init(const WindowProps& props = WindowProps());
		~Window();

		void OnUpdate();

		unsigned int GetWidth() const { return m_Data.Width; }
		unsigned int GetHeight() const { return m_Data.Height; }

		void SetEventCallback(const EventCallbackFn& callback) { m_Data.EventCallback = callback; }
		void SetVSync(bool enabled);
		bool IsVSync() const { return m_Data.VSync; }

		GLFWwindow* Get() { return m_Window; }
    private:
        GLFWwindow* m_Window;
		
        struct WindowData 
        {
			std::string Title;
			unsigned int Width, Height;
			bool VSync;
			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};
}
