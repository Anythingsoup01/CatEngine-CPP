#include "cepch.h"
#include "OpenGLShader.h"

#include <glad/glad.h>

namespace CatEngine
{
    OpenGLShader::OpenGLShader(const std::filesystem::path& vertShaderPath, const std::filesystem::path& fragShaderPath)
    {
        std::string vertCode = Utils::ReadFile(vertShaderPath);
        std::string fragCode = Utils::ReadFile(fragShaderPath);

        const char* vertCodeCStr = vertCode.c_str();
        const char* fragCodeCStr = fragCode.c_str();

        uint32_t vertShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertShader, 1, &vertCodeCStr, NULL);
        glCompileShader(vertShader);
        
        uint32_t fragShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragShader, 1, &fragCodeCStr, NULL);
        glCompileShader(fragShader);

        m_RendererID = glCreateProgram();
        glAttachShader(m_RendererID, vertShader);
        glAttachShader(m_RendererID, fragShader);
        glLinkProgram(m_RendererID);

        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
    }

    OpenGLShader::~OpenGLShader()
    {
        glDeleteProgram(m_RendererID);
    }

    void OpenGLShader::Bind() const
    {
        glUseProgram(m_RendererID);
    }

    void OpenGLShader::Unbind() const
    {
        glUseProgram(0);
    }
}
