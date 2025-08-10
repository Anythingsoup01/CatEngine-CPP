#pragma once

#include <glad/glad.h>
#include "CatEngine/Renderer/Texture.h"

namespace CatEngine
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const std::filesystem::path& path);
		~OpenGLTexture2D();

		virtual uint32_t GetWidth() const override { return m_Width; };
		virtual uint32_t GetHeight() const override { return m_Height; };

		virtual void Bind(uint32_t slot) const override;
    private:
        std::filesystem::path m_Path;
        uint32_t m_Width, m_Height;
        RendererID m_RendererID;
    };
}
