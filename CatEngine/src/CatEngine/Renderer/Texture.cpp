#include "cepch.h"
#include "Texture.h"

#include "RenderAPI.h"

#include "CatEngine/Platform/OpenGL/OpenGLTexture2D.h"

namespace CatEngine
{
    Ref<Texture2D> Texture2D::Create(const std::filesystem::path& filePath)
    {
        CE_PROFILE_FUNCTION();
        switch (RenderAPI::Get())
        {
            case RenderAPI::API::OpenGL: return CreateRef<OpenGLTexture2D>(filePath);
            case RenderAPI::API::Vulkan: CE_API_ASSERT(false, "Vulkan is currently unsupported!"); return nullptr;
            default: CE_API_ASSERT(false, "CatEngine Headless not supported!"); return nullptr;
        }
    }

    Ref<Texture2D> Texture2D::Create(const uint32_t& width, const uint32_t& height)
    {
        CE_PROFILE_FUNCTION();
        switch (RenderAPI::Get())
        {
            case RenderAPI::API::OpenGL: return CreateRef<OpenGLTexture2D>(width, height);
            case RenderAPI::API::Vulkan: CE_API_ASSERT(false, "Vulkan is currently unsupported!"); return nullptr;
            default: CE_API_ASSERT(false, "CatEngine Headless not supported!"); return nullptr;
        }
    }
}
