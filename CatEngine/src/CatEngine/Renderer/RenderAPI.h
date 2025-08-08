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
        static void Set(API api) { s_API = api; }

    private:
        static inline API s_API = API::None;
    };
}
