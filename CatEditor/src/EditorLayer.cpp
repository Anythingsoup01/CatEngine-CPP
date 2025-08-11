#include "EditorLayer.h"
#include "imgui.h" 

#include "glm/gtc/matrix_transform.hpp"
#include "glm/glm.hpp"

#include <glad/glad.h>

float vertices[8 * 4] = 
{
    // VERTEX POSITIONS   / VERTEX COLORS   / UV COORDS
    -0.50f, -0.50f, 0.00f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
     0.50f, -0.50f, 0.00f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
     0.50f,  0.50f, 0.00f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    -0.50f,  0.50f, 0.00f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
};

float vertices2[8 * 4] = 
{
    // VERTEX POSITIONS   / VERTEX COLORS   / UV COORDS
     0.50f,  0.50f, 0.00f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
     1.00f,  0.50f, 0.00f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
     1.00f,  1.00f, 0.00f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
     0.50f,  1.00f, 0.00f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
};

uint32_t indices[6] = 
{
    0, 1, 2,
    2, 3, 0
};

namespace CatEngine
{
    void EditorLayer::OnAttach()
    {
        s_Instance = this;

        Renderer::Init();
        m_CameraPos = Renderer::GetCameraPosition();

        m_TestVAO = VertexArray::Create();
        Ref<VertexBuffer> testVBO = VertexBuffer::Create(vertices, 8 * 4);
        testVBO->SetLayout({
            { ShaderDataType::Vec3 },
            { ShaderDataType::Vec3 },
            { ShaderDataType::Vec2 },
        });
        m_TestVAO->AddVertexBuffer(testVBO);
        Ref<IndexBuffer> testEBO = IndexBuffer::Create(indices, 6);
        m_TestVAO->SetIndexBuffer(testEBO);

        m_TestShader = Shader::Create("resources/shader/QuadShader.vert", "resources/shader/QuadShader.frag");

        m_Texture = Texture2D::Create("resources/Textures/car.png");
        m_Texture->Bind(0);

        m_TestTexture2 = Texture2D::Create("resources/Textures/car2.jpeg");
        m_TestTexture2->Bind(1);

    }

    void EditorLayer::OnDetach()
    {
        s_Instance = nullptr;
    }

    void EditorLayer::OnUpdate(Time deltaTime)
    {

        RenderCommand::Clear({0.24f, 0.16f, 0.78f, 1.0f});

        Renderer::BeginScene();
        
        glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.25f));

        m_TestShader->SetInt("u_TextureID", 0);

        for (int y = 0; y < m_GridSize[1]; y++)
        {
            for (int x = 0; x < m_GridSize[0]; x++)
            {
                glm::vec3 pos(x * 0.26f, y * 0.26f, 0.0f);
                glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
                Renderer::Submit(m_TestShader, m_TestVAO, transform);
            }
        }

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(0,-1,0)) * glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0,0,1.0f)) * 1.5f;

        m_TestShader->SetInt("u_TextureID", 1);
        Renderer::Submit(m_TestShader, m_TestVAO, transform);

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


        Renderer::SetCameraPosition(m_CameraPos);

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
