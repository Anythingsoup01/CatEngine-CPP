#include "cepch.h"
#include "Texture.h"

#include "RenderAPI.h"

#include "CatEngine/Platform/OpenGL/OpenGLTexture2D.h"

namespace CatEngine
{
    Ref<Texture2D> Texture2D::Create(const TextureSpecification& specification, Buffer data)
    {
        CE_PROFILE_FUNCTION();
        switch (RenderAPI::Get())
        {
            case RenderAPI::API::OpenGL: return CreateRef<OpenGLTexture2D>(specification, data);
            case RenderAPI::API::Vulkan: CE_API_ASSERT(false, "Vulkan is currently unsupported!"); return nullptr;
            default: CE_API_ASSERT(false, "CatEngine Headless not supported!"); return nullptr;
        }
    }
}
