#include "cepch.h"
#include "Texture.h"

#include "RenderAPI.h"

#include "CatEngine/Platform/OpenGL/OpenGLTexture2D.h"

#include <stb_image.h>

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

    Buffer TextureHelper::BufferTexture(const std::filesystem::path &filePath, int& width, int& height)
    {
        int channels;
        stbi_set_flip_vertically_on_load(1);
        Buffer data;
        {
            CE_PROFILE_SCOPE("stbi_load - TextureImporter::ImportTexture2D");
            data.Data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);
        }
        if (!data)
        {
            CE_API_ASSERT(false, "Failed to load texture from filepath {}", filePath.string());
            return {};
        }

        // TODO: Rework this when HDR gets supported!
        data.Size = width * height * channels;

        return data;

    }
}
