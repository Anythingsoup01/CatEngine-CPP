#include "cepch.h"
#include "GraphicsContext.h"

#include "RenderAPI.h"

#include "CatEngine/Platform/OpenGL/OpenGLGraphicsContext.h"

namespace CatEngine
{
    Scope<GraphicsContext> GraphicsContext::Create()
    {
        switch (RenderAPI::Get()) 
        {
            case RenderAPI::API::OpenGL:  return CreateScope<OpenGLGraphicsContext>();
            case RenderAPI::API::Vulkan:  CE_API_ASSERT(false, "Vulkan is currently unsupported!"); return nullptr;
            default: CE_API_ASSERT(false, "CatEngine Headless not supported!"); return nullptr;
        }
    }
}
