#pragma once

#include <glad/glad.h>
#include "CatEngine/Renderer/Texture.h"

namespace CatEngine
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const TextureSpecification& specification, Buffer data);
		~OpenGLTexture2D();

        virtual const TextureSpecification& GetSpecification() const override { return m_Specification; }

		virtual uint32_t GetWidth() const override { return m_Specification.Width; };
		virtual uint32_t GetHeight() const override { return m_Specification.Height; };
        virtual uint32_t GetRendererID() const override { return m_RendererID; }

		virtual void SetData(Buffer data) override;

		virtual void Bind(uint32_t slot) const override;

        virtual bool IsLoaded() const override { return m_IsLoaded; }

        virtual void RecreateTextureWithNewSpecification(const TextureSpecification& spec, Buffer data) override;

        virtual bool operator==(const Texture& other) const override
        {
            return m_RendererID == other.GetRendererID();
        }
    private:
        bool m_IsLoaded = false;
        GLenum m_InternalFormat, m_DataFormat;
        TextureSpecification m_Specification;
        RendererID m_RendererID;

    };
}
