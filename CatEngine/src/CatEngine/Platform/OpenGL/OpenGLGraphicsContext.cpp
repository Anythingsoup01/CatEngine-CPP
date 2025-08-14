#include "cepch.h"
#include "OpenGLGraphicsContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace CatEngine
{
    void OpenGLGraphicsContext::MakeContextCurrent(GLFWwindow* window)
    {
        CE_PROFILE_FUNCTION();
        CE_API_ASSERT(window, "Window Handle is null!");
        glfwMakeContextCurrent(window);

        m_CurrentWindow = window;
    }

    void OpenGLGraphicsContext::Init()
    {
        CE_PROFILE_FUNCTION();
        int status = gladLoadGL();
        CE_API_ASSERT(status, "Failed to load Glad!");

    }

    void OpenGLGraphicsContext::SwapBuffers()
    {
        CE_PROFILE_FUNCTION();
        glfwSwapBuffers(m_CurrentWindow);
    }
}
