#include "cepch.h"
#include "Renderer.h"

#include "RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>

#include "Renderer2D.h"

namespace CatEngine
{
    void Renderer::Init()
    {
        CE_PROFILE_FUNCTION();
        RenderCommand::Init();
        Renderer2D::Init();
    }

    void Renderer::Shutdown()
    {
        CE_PROFILE_FUNCTION();
        Renderer2D::Shutdown();
    }

    void Renderer::OnWindowResize(uint32_t width, uint32_t height)
    {
        CE_PROFILE_FUNCTION();
        RenderCommand::SetViewport(width, height);
    }

    void Renderer::BeginScene(OrthographicCamera& camera)
    {
        CE_PROFILE_FUNCTION();
        s_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
    }
    void Renderer::EndScene()
    {
    }

    void Renderer::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform)
    {
        CE_PROFILE_FUNCTION();
        shader->Bind();
        shader->SetMat4("u_ViewProjection", s_SceneData->ViewProjectionMatrix);
        shader->SetMat4("u_ModelMatrix", transform);

        vertexArray->Bind();
        RenderCommand::DrawIndexed(vertexArray);
    }
}
