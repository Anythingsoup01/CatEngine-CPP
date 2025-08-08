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

        static API Get() { return s_API; }
        static void Set(API api) { s_API = api; }

        virtual void Clear(const glm::vec4& clearColor) = 0;

        virtual void DrawIndexed(const Ref<VertexArray>& vertexArray) = 0;

        static Scope<RenderAPI> Create();
    private:
        static inline API s_API = API::None;
    };
}
