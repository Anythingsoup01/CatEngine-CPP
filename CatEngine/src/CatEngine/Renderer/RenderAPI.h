#pragma once

#include <glm/glm.hpp>
#include "VertexArray.h"

namespace CatEngine
{
    class RenderAPI
    {
    public:
        enum class API
        {
            None = 0,
            OpenGL, Vulkan
        };
    public:

        ~RenderAPI() = default;

        virtual void Init() = 0;
        virtual void SetViewport(uint32_t width, uint32_t height) = 0;
        virtual void Clear(const glm::vec4& clearColor) = 0;

        virtual void DrawIndexed(const Ref<VertexArray>& vertexArray) = 0;
        virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, const uint32_t& indexCount) = 0;

        static API Get() { return s_API; }
        static void Set(API api) { s_API = api; }
        static Scope<RenderAPI> Create();
    private:
        static inline API s_API = API::None;
    };
}
