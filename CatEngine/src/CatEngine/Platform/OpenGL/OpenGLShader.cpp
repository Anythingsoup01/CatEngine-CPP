#include "cepch.h"
#include "OpenGLShader.h"

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

namespace CatEngine
{
    OpenGLShader::OpenGLShader(const std::string& name, const std::filesystem::path& vertShaderPath, const std::filesystem::path& fragShaderPath)
        : m_Name(name)
    {
        CE_PROFILE_FUNCTION();
        std::string vertCode = Utils::ReadFile(vertShaderPath);
        std::string fragCode = Utils::ReadFile(fragShaderPath);

        const char* vertCodeCStr = vertCode.c_str();
        const char* fragCodeCStr = fragCode.c_str();

        uint32_t vertShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertShader, 1, &vertCodeCStr, NULL);
        glCompileShader(vertShader);

        GLint success;
        glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
        if (success == GL_FALSE)
        {
            GLint logLength;
            glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &logLength);

            std::vector<GLchar> infoLog(logLength);
            glGetShaderInfoLog(vertShader, logLength, NULL, &infoLog[0]);
            
            glDeleteShader(vertShader);

            CE_API_CRITICAL("VERTEX SHADER COMPILE ERROR: {}", infoLog.data());
        }
        
        uint32_t fragShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragShader, 1, &fragCodeCStr, NULL);
        glCompileShader(fragShader);

        glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
        if (success == GL_FALSE)
        {
            GLint logLength;
            glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &logLength);

            std::vector<GLchar> infoLog(logLength);
            glGetShaderInfoLog(fragShader, logLength, NULL, &infoLog[0]);

            glDeleteShader(fragShader);
            
            CE_API_CRITICAL("FRAGMENT SHADER COMPILE ERROR: {}", infoLog.data());
        }
        
        m_RendererID = glCreateProgram();
        glAttachShader(m_RendererID, vertShader);
        glAttachShader(m_RendererID, fragShader);
        glLinkProgram(m_RendererID);

        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
    }

    OpenGLShader::~OpenGLShader()
    {
        CE_PROFILE_FUNCTION();
        glDeleteProgram(m_RendererID);
    }

    void OpenGLShader::Bind() const
    {
        CE_PROFILE_FUNCTION();
        glUseProgram(m_RendererID);
    }

    void OpenGLShader::Unbind() const
    {
        CE_PROFILE_FUNCTION();
        glUseProgram(0);
    }

    void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& matrix)
    {
        CE_PROFILE_FUNCTION();
        glUseProgram(m_RendererID);
        glUniformMatrix4fv(CacheUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
    }

    void OpenGLShader::SetMat3(const std::string& name, const glm::mat3& matrix)
    {
        CE_PROFILE_FUNCTION();
        glUseProgram(m_RendererID);
        glUniformMatrix3fv(CacheUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
    }

    void OpenGLShader::SetMat2(const std::string& name, const glm::mat2& matrix)
    {
        CE_PROFILE_FUNCTION();
        glUseProgram(m_RendererID);
        glUniformMatrix2fv(CacheUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
    }

    void OpenGLShader::SetIntArray(const std::string& name, int* values, uint32_t count)
    {
        CE_PROFILE_FUNCTION();
        glUseProgram(m_RendererID);
        glUniform1iv(CacheUniformLocation(name), count, values);
    }

    void OpenGLShader::SetInt(const std::string& name, int value)
    {
        CE_PROFILE_FUNCTION();
        glUseProgram(m_RendererID);
        glUniform1i(CacheUniformLocation(name), value);
    }

    void OpenGLShader::SetVec4(const std::string& name, const glm::vec4& value)
    {
        CE_PROFILE_FUNCTION();
        glUseProgram(m_RendererID);
        glUniform4fv(CacheUniformLocation(name), 1, glm::value_ptr(value));
    }

    void OpenGLShader::SetVec3(const std::string& name, const glm::vec3& value)
    {
        CE_PROFILE_FUNCTION();
        glUseProgram(m_RendererID);
        glUniform3fv(CacheUniformLocation(name), 1, glm::value_ptr(value));
    }

    void OpenGLShader::SetVec2(const std::string& name, const glm::vec2& value)
    {
        CE_PROFILE_FUNCTION();
        glUseProgram(m_RendererID);
        glUniform2fv(CacheUniformLocation(name), 1, glm::value_ptr(value));
    }

    void OpenGLShader::SetFloat(const std::string& name, float value)
    {
        CE_PROFILE_FUNCTION();
        glUseProgram(m_RendererID);
        glUniform1f(CacheUniformLocation(name), value);
    }

    uint32_t OpenGLShader::CacheUniformLocation(const std::string& name)
    {
        CE_PROFILE_FUNCTION();
        auto it = m_UniformCache.find(name);
        if (it != m_UniformCache.end() && !m_UniformCache.empty())
            return it->second;

        uint32_t location = glGetUniformLocation(m_RendererID, name.c_str());
        m_UniformCache.emplace(std::pair<std::string, uint32_t>(name, location));
        return location;
    }
}
