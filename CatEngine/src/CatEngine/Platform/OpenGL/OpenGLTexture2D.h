#pragma once

#include <glad/glad.h>
#include "CatEngine/Renderer/Texture.h"

namespace CatEngine
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const std::filesystem::path& path);
		OpenGLTexture2D(const uint32_t& width, const uint32_t& height);
		~OpenGLTexture2D();

		virtual uint32_t GetWidth() const override { return m_Width; };
		virtual uint32_t GetHeight() const override { return m_Height; };

		virtual void SetData(void* data, uint32_t size) override;

		virtual void Bind(uint32_t slot) const override;

        virtual bool IsLoaded() const override { return m_IsLoaded; }

        virtual bool operator==(const Texture& other) const override
        {
            return m_RendererID == other.GetRendererID();
        }

        virtual uint32_t GetRendererID() const override { return m_RendererID; }
    private:
        std::filesystem::path m_Path;
        bool m_IsLoaded = false;
        uint32_t m_Width, m_Height;
        GLenum m_InternalFormat, m_Format;
        RendererID m_RendererID;
    };
}
