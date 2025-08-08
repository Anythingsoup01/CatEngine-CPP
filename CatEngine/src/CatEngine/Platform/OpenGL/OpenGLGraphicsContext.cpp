#include "cepch.h"
#include "OpenGLGraphicsContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace CatEngine
{
    void OpenGLGraphicsContext::MakeContextCurrent(GLFWwindow* window)
    {
        CE_API_ASSERT(window, "Window Handle is null!");
        glfwMakeContextCurrent(window);

        m_CurrentWindow = window;
    }

    void OpenGLGraphicsContext::Init()
    {
        int status = gladLoadGL();
        CE_API_ASSERT(status, "Failed to load Glad!");
    }

    void OpenGLGraphicsContext::SwapBuffers()
    {
        glfwSwapBuffers(m_CurrentWindow);
    }
}
