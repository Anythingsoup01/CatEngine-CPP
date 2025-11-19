#include "CatEngine/Renderer/Texture.h"
#include "cepch.h"
#include "OpenGLTexture2D.h"


namespace CatEngine
{
    namespace OpenGLTextureUtils
    {
		static GLenum ImageFormatToGLDataFormat(ImageFormat format)
		{
			switch (format)
			{
                case ImageFormat::R8:    return GL_R8;
				case ImageFormat::RGB8:  return GL_RGB;
				case ImageFormat::RGBA8: return GL_RGBA;
			}

		    CE_API_ASSERT(false, "Image Format Not Supported!");
			return 0;
		}
		
		static GLenum ImageFormatToGLInternalFormat(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::RGB8:  return GL_RGB8;
			case ImageFormat::RGBA8: return GL_RGBA8;
			}

		    CE_API_ASSERT(false, "Image Format Not Supported!");
			return 0;
		}
        static ImageFormat GLInternalFormatToImageFormat(GLenum type)
		{
			switch (type)
			{
                case GL_RGB8:  return ImageFormat::RGB8;
                case GL_RGBA8: return ImageFormat::RGBA8;
			}

		    CE_API_ASSERT(false, "GL Image Format Not Supported!");
		}

        static GLenum TextureParameterToOpenGLTextureParameter(const TextureParameter& param)
        {
            switch (param) 
            {
                case TextureParameter::Linear: return GL_LINEAR;
                case TextureParameter::Nearest: return GL_NEAREST;
                default: break;
            }

            CE_API_ASSERT(false, "TextureParameter Not supported!");
            return 0;
        }

        static GLenum TextureWrapToOpenGLTextureParameter(const TextureWrap& param)
        {
            switch (param) 
            {
                case TextureWrap::Repeat: return GL_REPEAT;
                case TextureWrap::MirroredRepeat: return GL_MIRRORED_REPEAT;
                case TextureWrap::ClampToEdge: return GL_CLAMP_TO_EDGE;
                case TextureWrap::ClampToBorders: return GL_CLAMP_TO_BORDER;
                default: break;
            }

            CE_API_ASSERT(false, "TextureParameter Not supported!");
            return 0;
        }
    }

    OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification& specification, Buffer data)
        : m_Specification(specification)
    {
        CE_PROFILE_FUNCTION();
        
        m_MetaData = CreateRef<TextureMetaData>();

        m_MetaData->TextureMinFilterIndex = (int)specification.MinFilter;
        m_MetaData->TextureMagFilterIndex = (int)specification.MagFilter;
        m_MetaData->TextureWrapIndex = (int)specification.WrapOption;
        std::filesystem::path fileName = specification.FilePath.filename();
        fileName.replace_extension("");
        m_MetaData->AssetName = fileName.generic_string();
        m_MetaData->Type = AssetType::Texture2D;
        m_MetaData->FilePath = specification.FilePath;

        m_IsLoaded = true;

        m_InternalFormat = OpenGLTextureUtils::ImageFormatToGLInternalFormat(m_Specification.Format);
        m_DataFormat = OpenGLTextureUtils::ImageFormatToGLDataFormat(m_Specification.Format);

        glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
        glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Specification.Width, m_Specification.Height);

        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, OpenGLTextureUtils::TextureParameterToOpenGLTextureParameter(specification.MinFilter));
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, OpenGLTextureUtils::TextureParameterToOpenGLTextureParameter(specification.MagFilter));

        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, OpenGLTextureUtils::TextureWrapToOpenGLTextureParameter(specification.WrapOption));
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, OpenGLTextureUtils::TextureWrapToOpenGLTextureParameter(specification.WrapOption));

        if (data)
            SetData(data);

    }

    void OpenGLTexture2D::SetData(Buffer data)
    {
        CE_PROFILE_FUNCTION();
        uint32_t bpc = m_Specification.Format == ImageFormat::RGBA8 ? 4 : 3;
        CE_API_ASSERT(data.Size == m_Specification.Width * m_Specification.Height * bpc, "Data must be entire texture");
        glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Specification.Width, m_Specification.Height, m_DataFormat, GL_UNSIGNED_BYTE, data.Data);
    }

    void OpenGLTexture2D::RecreateTextureWithNewSpecification(const TextureSpecification& spec, Buffer data)
    {
        CE_PROFILE_FUNCTION();

        m_IsLoaded = true;

        m_InternalFormat = OpenGLTextureUtils::ImageFormatToGLInternalFormat(m_Specification.Format);
        m_DataFormat = OpenGLTextureUtils::ImageFormatToGLDataFormat(m_Specification.Format);

        glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
        glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Specification.Width, m_Specification.Height);

        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, OpenGLTextureUtils::TextureParameterToOpenGLTextureParameter(spec.MinFilter));
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, OpenGLTextureUtils::TextureParameterToOpenGLTextureParameter(spec.MagFilter));

        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, OpenGLTextureUtils::TextureWrapToOpenGLTextureParameter(spec.WrapOption));
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, OpenGLTextureUtils::TextureWrapToOpenGLTextureParameter(spec.WrapOption));

        if (data)
            SetData(data);

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
