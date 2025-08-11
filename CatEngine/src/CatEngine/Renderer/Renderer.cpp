#include "cepch.h"
#include "Renderer.h"

#include "RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>

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

    void Renderer::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform)
    {
        shader->Bind();
        shader->SetMat4("u_ViewProjection", m_Camera.GetViewProjectionMatrix());
        shader->SetMat4("u_ModelMatrix", transform);

        vertexArray->Bind();
        RenderCommand::DrawIndexed(vertexArray);
    }
}
