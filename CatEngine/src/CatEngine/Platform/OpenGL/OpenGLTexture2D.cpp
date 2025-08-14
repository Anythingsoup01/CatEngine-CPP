#include "cepch.h"
#include "OpenGLTexture2D.h"

#include <stb_image.h>

namespace CatEngine
{
    OpenGLTexture2D::OpenGLTexture2D(const std::filesystem::path& filePath)
        : m_Path(filePath)
    {
        CE_PROFILE_FUNCTION();
        int width, height, channels;
        stbi_set_flip_vertically_on_load(1);
        stbi_uc* data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);

        if (data)
        {
            m_IsLoaded = true;
            m_Width = width;
            m_Height = height;

            GLenum internalFormat = 0, dataFormat = 0;

            if (channels == 4)
            {
                internalFormat = GL_RGBA8;
                dataFormat = GL_RGBA;
            }
            else if (channels == 3)
            {
                internalFormat = GL_RGB8;
                dataFormat = GL_RGB;
            }

            CE_API_ASSERT(internalFormat & dataFormat, "Format not supported!");

            glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
            glTextureStorage2D(m_RendererID, 1, internalFormat, width, height);

            glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);

            glTextureSubImage2D(m_RendererID, 0, 0, 0, width, height, dataFormat, GL_UNSIGNED_BYTE, data);

            stbi_image_free(data);
        }
    }

    OpenGLTexture2D::OpenGLTexture2D(const uint32_t& width, const uint32_t& height)
        : m_Width(width), m_Height(height), m_Path("DEFAULTTEXTURE")
    {
        CE_PROFILE_FUNCTION();

        m_IsLoaded = true;
        m_Width = width;
        m_Height = height;

        m_InternalFormat = GL_RGBA8;
        m_Format = GL_RGBA;

        glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
        glTextureStorage2D(m_RendererID, 1, m_InternalFormat, width, height);

        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
    }

    void OpenGLTexture2D::SetData(void* data, uint32_t size)
    {
        CE_PROFILE_FUNCTION();
        uint32_t bpc = m_Format == GL_RGBA ? 4 : 3;
        CE_API_ASSERT(size == m_Width * m_Height * bpc, "Data must be entire texture");
        glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_Format, GL_UNSIGNED_BYTE, data);
    }


    OpenGLTexture2D::~OpenGLTexture2D()
    {
        CE_PROFILE_FUNCTION();
        glDeleteTextures(1, &m_RendererID);
    }

    void OpenGLTexture2D::Bind(uint32_t slot) const
    {
        CE_PROFILE_FUNCTION();
        glBindTextureUnit(slot, m_RendererID);
    }
}
