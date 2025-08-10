#include "EditorLayer.h"
#include "imgui.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/glm.hpp"


float redVertices[8 * 4] =
{    // Vertex Positions //  Color Values  // Texture Coords
     -0.50f, -0.50f, 0.0f, 1.0f, 0.0f, 0.0f,       0, 0, // 0 
      0.50f, -0.50f, 0.0f, 1.0f, 0.0f, 0.0f,       1, 0, // 1
      0.50f,  0.50f, 0.0f, 1.0f, 0.0f, 0.0f,       1, 1, // 2
     -0.50f,  0.50f, 0.0f, 1.0f, 0.0f, 0.0f,       0, 1. // 3
};

uint32_t indices[6]
{
    0, 1, 2,
    2, 3, 0,
};

namespace CatEngine
{

    void EditorLayer::OnAttach()
    {
        s_Instance = this;

        Renderer::Init();

        m_Camera.SetProjection(-1.6f, 1.6f, -0.9f, 0.9f);

        m_RedVAO = VertexArray::Create();
        m_RedVBO = VertexBuffer::Create(redVertices, 6 * 8);
        m_RedVBO->SetLayout({
            { ShaderDataType::Vec3 },
            { ShaderDataType::Vec3 },
            { ShaderDataType::Vec2 },
        });
        m_RedVAO->AddVertexBuffer(m_RedVBO);
        m_RedEBO = IndexBuffer::Create(indices, 12);
        m_RedVAO->SetIndexBuffer(m_RedEBO);

        // Shader
        m_Shader = Shader::Create("resources/shader/QuadShader.vert", "resources/shader/QuadShader.frag");
        m_Shader->Bind();

    }

    void EditorLayer::OnDetach()
    {
        s_Instance = nullptr;
    }

    void EditorLayer::OnUpdate(Time deltaTime)
    {

        RenderCommand::Clear({0.24f, 0.16f, 0.78f, 1.0f});

        Renderer::BeginScene(m_Camera);

        glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

        for (int y = 0; y < m_GridSize[1]; y++)
        {
            for (int x = 0; x < m_GridSize[0]; x++)
            {
                glm::vec3 pos = glm::vec3(x * 0.11f, y * 0.11f, 0.0f);
                glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
                Renderer::Submit(m_Shader, m_RedVAO, transform);
            }
        }

        Renderer::EndScene();
        
        if (Input::IsKeyPressed(KeyCode::W))
            m_CameraPos.y += 1.0f * deltaTime;

        if (Input::IsKeyPressed(KeyCode::S))
            m_CameraPos.y -= 1 * deltaTime;

        if (Input::IsKeyPressed(KeyCode::A))
            m_CameraPos.x -= 1 * deltaTime;

        if (Input::IsKeyPressed(KeyCode::D))
            m_CameraPos.x += 1 * deltaTime;

        m_DeltaTime = deltaTime;


        m_Camera.SetPosition(m_CameraPos);

    }


    void EditorLayer::OnImGuiDraw()
    {
        ImGui::Begin("Stats");
        ImGui::Text("Delta Time: %f", m_DeltaTime);
        ImGui::Text("Rough FPS: %f", 1.0f / m_DeltaTime);
        ImGui::DragInt2("Grid Size", m_GridSize, 0.5f, 1, 1000);
        ImGui::End();
    }

    void EditorLayer::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);

    }
}
