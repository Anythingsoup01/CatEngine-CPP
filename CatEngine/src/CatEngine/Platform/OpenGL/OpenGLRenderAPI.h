#pragma once
#include "CatEngine/Renderer/RenderAPI.h"

namespace CatEngine
{
    class OpenGLRenderAPI : public RenderAPI
    {
    public:
        virtual void Init() override;
        virtual void SetViewport(uint32_t width, uint32_t height) override;
        virtual void Clear(const glm::vec4& clearColor) override;
        virtual void DrawIndexed(const Ref<VertexArray>& vertexArray) override;
        virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, const uint32_t& indexCount) override;
    };
}
