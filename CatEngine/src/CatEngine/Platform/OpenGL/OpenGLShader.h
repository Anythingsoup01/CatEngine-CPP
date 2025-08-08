#pragma once
#include "CatEngine/Renderer/Shader.h"

namespace CatEngine
{
    class OpenGLShader : public Shader
    {
    public:
        OpenGLShader(const std::filesystem::path& vertShaderPath, const std::filesystem::path& fragShaderPath);
        ~OpenGLShader();

        virtual void Bind() const override;
        virtual void Unbind() const override;

    private:
        RendererID m_RendererID;
    };
}
