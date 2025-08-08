#pragma once
#include "CatEngine/Renderer/GraphicsContext.h"


namespace CatEngine
{
    class OpenGLGraphicsContext : public GraphicsContext
    {
    public:
        virtual void MakeContextCurrent(GLFWwindow* window) override;
        virtual void Init() override;
        virtual void SwapBuffers() override;
    private:
        GLFWwindow* m_CurrentWindow;
    };
}
