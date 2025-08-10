#pragma once
#include "CatEngine/Renderer/Shader.h"

#include <unordered_map>

namespace CatEngine
{
    class OpenGLShader : public Shader
    {
    public:
        OpenGLShader(const std::filesystem::path& vertShaderPath, const std::filesystem::path& fragShaderPath);
        ~OpenGLShader();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual void SetMat4(const std::string& name, const glm::mat4& matrix) override;
        virtual void SetMat3(const std::string& name, const glm::mat3& matrix) override;
        virtual void SetMat2(const std::string& name, const glm::mat2& matrix) override;

        virtual void SetIntArray(const std::string& name, int* values, uint32_t count) override;
        virtual void SetInt(const std::string& name, int value) override;

        virtual void SetVec4(const std::string& name, const glm::vec4& value) override;
        virtual void SetVec3(const std::string& name, const glm::vec3& value) override;
        virtual void SetVec2(const std::string& name, const glm::vec2& value) override;
        
        virtual void SetFloat(const std::string& name, float value) override;
    private:
        uint32_t CacheUniformLocation(const std::string& name);

    private:
        RendererID m_RendererID;

        std::unordered_map<std::string, uint32_t> m_UniformCache;
    };
}
