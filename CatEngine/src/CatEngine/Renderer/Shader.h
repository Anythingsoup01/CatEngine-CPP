#pragma once

#include <glm/glm.hpp>

namespace CatEngine
{
    class Shader
    {
    public:
        ~Shader() = default;

        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        virtual void SetMat4(const std::string& name, const glm::mat4& matrix) = 0;
        virtual void SetMat3(const std::string& name, const glm::mat3& matrix) = 0;
        virtual void SetMat2(const std::string& name, const glm::mat2& matrix) = 0;

        virtual void SetIntArray(const std::string& name, int* values, uint32_t count) = 0;
        virtual void SetInt(const std::string& name, int value) = 0;

        virtual void SetVec4(const std::string& name, const glm::vec4& value) = 0;
        virtual void SetVec3(const std::string& name, const glm::vec3& value) = 0;
        virtual void SetVec2(const std::string& name, const glm::vec2& value) = 0;
        
        virtual void SetFloat(const std::string& name, float value) = 0;

        virtual std::string GetName() const = 0;
        
        static Ref<Shader> Create(const std::string& name, const std::filesystem::path& vertShaderPath, const std::filesystem::path& fragShaderPath);
    };

    class ShaderLibrary
    {
    public:
        void Add(const Ref<Shader>& shader);
        void Add(const std::string& name, const Ref<Shader>& shader);

        Ref<Shader> Load(const std::string& name, const std::filesystem::path& vertShaderPath, const std::filesystem::path& fragShaderPath);
        Ref<Shader> Get(const std::string& name);

        bool Exists(const std::string& name) const { return m_Shaders.find(name) != m_Shaders.end(); }
    private:
        std::unordered_map<std::string, Ref<Shader>> m_Shaders;
    };
}
