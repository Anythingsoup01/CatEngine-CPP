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
            vertexArray->Bind();
            s_RenderAPI->DrawIndexed(vertexArray);
        }

        static inline void DrawIndexed(const Ref<VertexArray>& vertexArray, const uint32_t& indexCount)
        {
            vertexArray->Bind();
            s_RenderAPI->DrawIndexed(vertexArray, indexCount);
        }

        static inline void DrawLines(const Ref<VertexArray>& vertexArray, const uint32_t vertexCount)
        {
            vertexArray->Bind();
            s_RenderAPI->DrawLines(vertexArray, vertexCount);
        }

        static inline void Clear(const glm::vec4& clearColor)
        {
            s_RenderAPI->Clear(clearColor);
        }

        static inline void SetLineThickness(float thickness)
        {
            s_RenderAPI->SetLineThickness(thickness);
        }
    private:
        static inline Scope<RenderAPI> s_RenderAPI = nullptr;
    };
}
