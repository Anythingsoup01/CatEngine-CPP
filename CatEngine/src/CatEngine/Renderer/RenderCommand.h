#pragma once

#include "RenderAPI.h"
#include "VertexArray.h"

namespace CatEngine
{
    class RenderCommand
    {
    public:
        static void Init();

        static inline void DrawIndexed(const Ref<VertexArray>& vertexArray)
        {
            s_RenderAPI->DrawIndexed(vertexArray);
        }

        static inline void Clear(const glm::vec4& clearColor)
        {
            s_RenderAPI->Clear(clearColor);
        }
    private:
        static inline Scope<RenderAPI> s_RenderAPI = nullptr;
    };
}
