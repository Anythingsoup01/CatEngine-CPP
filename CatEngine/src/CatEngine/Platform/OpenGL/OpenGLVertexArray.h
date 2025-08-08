#pragma once
#include "CatEngine/Renderer/VertexArray.h"

namespace CatEngine
{
    class OpenGLVertexArray : public VertexArray
    {
    public:
        OpenGLVertexArray();
        ~OpenGLVertexArray();

        virtual void Bind() const override;
        virtual void Unbind() const override;
    private:
        RendererID m_RendererID;
    };
}
