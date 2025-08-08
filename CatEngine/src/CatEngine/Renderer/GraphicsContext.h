#pragma once

#include <GLFW/glfw3.h>

namespace CatEngine
{
    class GraphicsContext
    {
    public:
        ~GraphicsContext() = default;

        virtual void MakeContextCurrent(GLFWwindow* window) = 0;
        virtual void Init() = 0;
        virtual void SwapBuffers() = 0;

        static Scope<GraphicsContext> Create();
    };
}
