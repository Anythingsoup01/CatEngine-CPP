#pragma once

#include "RenderAPI.h"
#include "VertexArray.h"

namespace CatEngine
{
    class RenderCommand
    {
    public:
        static void Init();

        static void SetViewport(uint32_t width, uint32_t height) { s_RenderAPI->SetViewport(width, height); }

        static inline void DrawIndexed(const Ref<VertexArray>& vertexArray)
        {
            s_RenderAPI->DrawIndexed(vertexArray);
        }

        static inline void DrawIndexed(const Ref<VertexArray>& vertexArray, const uint32_t& indexCount)
        {
            s_RenderAPI->DrawIndexed(vertexArray, indexCount);
        }

        static inline void Clear(const glm::vec4& clearColor)
        {
            s_RenderAPI->Clear(clearColor);
        }
    private:
        static inline Scope<RenderAPI> s_RenderAPI = nullptr;
    };
}
