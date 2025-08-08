#pragma once

#include "VertexArray.h"

namespace CatEngine
{
    class Renderer
    {
    public:
        static void Init();

        static void BeginScene();
        static void EndScene();

        static void Submit(const Ref<VertexArray>& vertexArray);

    private:

    };
}
