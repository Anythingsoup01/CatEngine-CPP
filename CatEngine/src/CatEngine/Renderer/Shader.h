#pragma once

namespace CatEngine
{
    class Shader
    {
    public:
        ~Shader() = default;

        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;
        
        static Scope<Shader> Create(const std::filesystem::path& vertShaderPath, const std::filesystem::path& fragShaderPath);
    };
}
