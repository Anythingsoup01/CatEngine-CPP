#include "cepch.h"
#include "Shader.h"

#include "RenderAPI.h"

#include "CatEngine/Platform/OpenGL/OpenGLShader.h"

namespace CatEngine
{
    Ref<Shader> Shader::Create(const std::filesystem::path& vertShaderPath, const std::filesystem::path& fragShaderPath)
    {
        switch (RenderAPI::Get())
        {
            case RenderAPI::API::OpenGL: return CreateRef<OpenGLShader>(vertShaderPath, fragShaderPath);
            case RenderAPI::API::Vulkan: CE_API_ASSERT(false, "Vulkan is currently unsupported!"); return nullptr;
            default: CE_API_ASSERT(false, "CatEngine Headless not supported!");
        }
    }
}
