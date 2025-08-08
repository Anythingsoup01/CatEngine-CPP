#include "cepch.h"
#include "Renderer.h"

#include "RenderCommand.h"

namespace CatEngine
{

    void Renderer::Init()
    {
        RenderCommand::Init();
    }

    void Renderer::BeginScene()
    {

    }
    void Renderer::EndScene()
    {

    }

    void Renderer::Submit(const Ref<VertexArray>& vertexArray)
    {
        RenderCommand::DrawIndexed(vertexArray);
    }
}
