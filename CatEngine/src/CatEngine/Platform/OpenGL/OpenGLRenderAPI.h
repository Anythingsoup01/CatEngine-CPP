#pragma once
#include "CatEngine/Renderer/RenderAPI.h"

namespace CatEngine
{
    class OpenGLRenderAPI : public RenderAPI
    {
    public:
        virtual void Clear(const glm::vec4& clearColor) override;
        virtual void DrawIndexed(const Ref<VertexArray>& vertexArray) override;
    };
}
