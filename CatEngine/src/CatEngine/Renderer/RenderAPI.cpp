#include "cepch.h"
#include "RenderAPI.h"

#include "CatEngine/Platform/OpenGL/OpenGLRenderAPI.h"

namespace CatEngine
{
    Scope<RenderAPI> RenderAPI::Create()
    {
        switch (s_API)
        {
            case API::OpenGL: return CreateScope<OpenGLRenderAPI>();
            case API::Vulkan: CE_API_ASSERT(false, "Vulkan is currently unsupported!"); return nullptr;
            default: CE_API_ASSERT(false, "CatEngine Headless is unsupported!"); return nullptr;
        }
    }
}
