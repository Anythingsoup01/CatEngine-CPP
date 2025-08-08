#pragma once

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

        static API Get() { return s_API; }

    private:
        static inline API s_API = API::OpenGL;
    };
}
