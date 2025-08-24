#include "cepch.h"
#include "Renderer2D.h"

#include "VertexArray.h"
#include "Buffer.h"
#include "Shader.h"
#include "UniformBuffer.h"

#include "RenderCommand.h"

// TODO REMOVE
#include <glm/gtc/matrix_transform.hpp>

namespace CatEngine
{

    struct QuadVertex
    {
        glm::vec3 Position;
        glm::vec4 Color;
        glm::vec2 TexCoord;
        float TexIndex;
        float TilingFactor;
        int EntityID;
    };

    struct Renderer2DData
    {
        static const uint32_t MaxQuads = 1000;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;
		static const uint32_t MaxTextureSlots = 32; // TODO: RenderCaps
        
        Ref<VertexArray> QuadVertexArray;
        Ref<VertexBuffer> QuadVertexBuffer;

        uint32_t QuadIndexCount = 0;
        QuadVertex* QuadVertexBufferBase = nullptr;
        QuadVertex* QuadVertexBufferPtr = nullptr;

        glm::vec4 QuadVertexPositions[4];

        std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
        Ref<Texture2D> DefaultTexture; // Bound to slot 0 
        uint32_t TextureSlotIndex = 1;

        struct CameraData
        {
            glm::mat4 ViewProjection;
        };
        CameraData CameraBuffer;
        Ref<UniformBuffer> CameraUniformBuffer;

        ShaderLibrary Shaders;

        Renderer2D::Statistics Stats;
    };

    static Renderer2DData s_Data;

    
    void Renderer2D::Init()
    {
        CE_PROFILE_FUNCTION();
        s_Data.QuadVertexArray = VertexArray::Create();
        s_Data.QuadVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex));
        s_Data.QuadVertexBuffer->SetLayout({
            { ShaderDataType::Vec3, "a_Position"     },
            { ShaderDataType::Vec4, "a_Color"        },
            { ShaderDataType::Vec2, "a_TexCoord"     },
            { ShaderDataType::Vec,  "a_TexIndex"     },
            { ShaderDataType::Vec,  "a_TilingFactor" },
            { ShaderDataType::Int,  "a_EntityID"     },
        });
        s_Data.QuadVertexArray->AddVertexBuffer(s_Data.QuadVertexBuffer);

        uint32_t* quadIndices = new uint32_t[s_Data.MaxIndices];

        uint32_t offset = 0;
        for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6)
        {
            quadIndices[i + 0] = offset + 0;
            quadIndices[i + 1] = offset + 1;
            quadIndices[i + 2] = offset + 2;

            quadIndices[i + 3] = offset + 2;
            quadIndices[i + 4] = offset + 3;
            quadIndices[i + 5] = offset + 0;

            offset += 4;
        }

        Ref<IndexBuffer> quadIndexBuffer = IndexBuffer::Create(quadIndices, s_Data.MaxIndices);
        s_Data.QuadVertexArray->SetIndexBuffer(quadIndexBuffer);

        delete[] quadIndices; // Can cause issues later with referece depending on thread

        s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];

        s_Data.QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
        s_Data.QuadVertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
        s_Data.QuadVertexPositions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
        s_Data.QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

        s_Data.Shaders.Load("QuadShader", "Resources/Shader/2D/QuadShader.vert", "Resources/Shader/2D/QuadShader.frag");

        int32_t samplers[s_Data.MaxTextureSlots];
        for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
            samplers[i] = i;
        s_Data.Shaders.Get("QuadShader")->SetIntArray("u_Textures", samplers, s_Data.MaxTextureSlots);

        s_Data.DefaultTexture = Texture2D::Create(1, 1);
        uint32_t data = 0xffffffff;
        s_Data.DefaultTexture->SetData(&data, sizeof(uint32_t));
        
        s_Data.TextureSlots[0] = s_Data.DefaultTexture;
        for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
            s_Data.TextureSlots[i] = s_Data.DefaultTexture;


        s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer2DData::CameraBuffer), 0);


    }

    void Renderer2D::Shutdown()
    {
        CE_PROFILE_FUNCTION();
        
    }
	
    void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		CE_PROFILE_FUNCTION();

        if (m_SceneActive) CE_API_ASSERT(false, "No end scene!");

        m_SceneActive = true;
        
        s_Data.CameraBuffer.ViewProjection = camera.GetProjection() * glm::inverse(transform);
        s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer2DData::CameraBuffer));

		StartBatch();
	}

	void Renderer2D::BeginScene(const EditorCamera& camera)
	{
		CE_PROFILE_FUNCTION();
        
        if (m_SceneActive) CE_API_ASSERT(false, "No end scene!");

        m_SceneActive = true;

        s_Data.CameraBuffer.ViewProjection = camera.GetViewProjection();
        s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer2DData::CameraBuffer));

		StartBatch();
	}

    void Renderer2D::EndScene()
    {
        CE_PROFILE_FUNCTION();

        if (!m_SceneActive) CE_API_ASSERT(false, "No begin scene!");

        m_SceneActive = false;
        Flush();
    }

    void Renderer2D::StartBatch()
    {
        s_Data.QuadIndexCount = 0;
        s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

        s_Data.TextureSlotIndex = 1;
    }

    void Renderer2D::Flush()
    {
        if (s_Data.QuadIndexCount)
        {
            uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase); 
            s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);

            for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
                s_Data.TextureSlots[i]->Bind(i);

            s_Data.Shaders.Get("QuadShader")->Bind();
            RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
            s_Data.Stats.DrawCalls++;
        }
    }

    void Renderer2D::NextBatch()
    {
        Flush();
        StartBatch();
    }

    void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, const Ref<Texture2D>& texture, float tilingFactor, int entityID)
    {
        CE_PROFILE_FUNCTION();
        if (!m_SceneActive) CE_API_ASSERT(false, "No begin scene!");
        float textureIndex = 0.0f;
        constexpr glm::vec2 textureCoords[4] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };

        if (texture)
        {
            for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
            {
                if (*s_Data.TextureSlots[i] == *texture.get())
                {
                    textureIndex = (float)i;
                    break;
                }
            }

            if (textureIndex == 0.0f)
            {
                textureIndex = (float)s_Data.TextureSlotIndex;
                s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
                s_Data.TextureSlotIndex++;
            }
        }

        if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
            NextBatch();


        for (size_t i = 0; i < 4; i++)
        {
            s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->Color = color;
            s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
            s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
            s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
            s_Data.QuadVertexBufferPtr->EntityID = entityID;

            s_Data.QuadVertexBufferPtr++;
        }
        
        s_Data.QuadIndexCount += 6;
        s_Data.Stats.QuadCount++;
    }

    void Renderer2D::ResetStats()
    {
        memset(&s_Data.Stats, 0, sizeof(Statistics));
    }

    Renderer2D::Statistics Renderer2D::GetStats()
    {
        return s_Data.Stats;
    }
}
