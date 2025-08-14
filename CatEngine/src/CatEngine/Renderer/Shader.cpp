#include "cepch.h"
#include "Shader.h"

#include "RenderAPI.h"

#include "CatEngine/Platform/OpenGL/OpenGLShader.h"

namespace CatEngine
{
    Ref<Shader> Shader::Create(const std::string& name, const std::filesystem::path& vertShaderPath, const std::filesystem::path& fragShaderPath)
    {
        CE_PROFILE_FUNCTION();
        switch (RenderAPI::Get())
        {
            case RenderAPI::API::OpenGL: return CreateRef<OpenGLShader>(name, vertShaderPath, fragShaderPath);
            case RenderAPI::API::Vulkan: CE_API_ASSERT(false, "Vulkan is currently unsupported!"); return nullptr;
            default: CE_API_ASSERT(false, "CatEngine Headless not supported!"); return nullptr;
        }
    }

    void ShaderLibrary::Add(const Ref<Shader>& shader)
    {
        CE_PROFILE_FUNCTION();
        Add(shader->GetName(), shader);
    }
    
    void ShaderLibrary::Add(const std::string& name, const Ref<Shader>& shader)
    {
        CE_PROFILE_FUNCTION();
        if (Exists(name))
        {
            CE_API_ERROR("Shader already exists in shader library!");
            return;
        }

        m_Shaders.emplace(std::pair<std::string, Ref<Shader>>(name, shader));

    }
    
    Ref<Shader> ShaderLibrary::Load(const std::string& name, const std::filesystem::path& vertPath, const std::filesystem::path& fragPath)
    {
        CE_PROFILE_FUNCTION();
        Ref<Shader> shader = Shader::Create(name, vertPath, fragPath);
        Add(name, shader);
        return shader;
    }
    
    Ref<Shader> ShaderLibrary::Get(const std::string& name)
    {
        CE_PROFILE_FUNCTION();
        return Exists(name) ? m_Shaders.at(name) : nullptr;
    }
}
