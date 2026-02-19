#include "Renderer3D.h"
#include "cepch.h"

#include "Buffer.h"
#include "Shader.h"
#include "UniformBuffer.h"
#include "VertexArray.h"

#include "RenderCommand.h"

#include "CatEngine/AssetManager/AssetManager.h"

#include "Texture.h"

#include "RenderTypes.h"

namespace CatEngine {

struct Renderer3DData {
  static const uint32_t MaxCubes = 500 * 6;
  static const uint32_t MaxVertices = MaxCubes * 24;
  static const uint32_t MaxIndices = MaxCubes * 6;
  static const uint32_t MaxTextureSlots = 32;

  Ref<VertexArray> CubeVertexArray;
  Ref<VertexBuffer> CubeVertexBuffer;

  uint32_t CubeIndexCount = 0;
  QuadVertex *CubeVertexBufferBase = nullptr;
  QuadVertex *CubeVertexBufferPtr = nullptr;

  glm::vec4 CubeVertexPositions[24];

  std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
  Ref<Texture2D> DefaultTexture; // Bound to slot 0
  uint32_t TextureSlotIndex = 1;

  struct CameraData {
    glm::mat4 ViewProjection;
  };
  CameraData CameraBuffer;
  Ref<UniformBuffer> CameraUniformBuffer;

  ShaderLibrary Shaders;

  // Renderer3D::Statistics Stats;
};

static Renderer3DData s_Data;

void Renderer3D::Init() {
  CE_PROFILE_FUNCTION();
  s_Data.CubeVertexArray = VertexArray::Create();
  s_Data.CubeVertexBuffer =
      VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex));
  s_Data.CubeVertexBuffer->SetLayout({
      {ShaderDataType::Vec3, "a_Position"},
      {ShaderDataType::Vec4, "a_Color"},
      {ShaderDataType::Vec2, "a_TexCoord"},
      {ShaderDataType::Vec, "a_TexIndex"},
      {ShaderDataType::Vec, "a_TilingFactor"},
      {ShaderDataType::Int, "a_EntityID"},
  });
  s_Data.CubeVertexArray->AddVertexBuffer(s_Data.CubeVertexBuffer);

  uint32_t *cubeIndices = new uint32_t[s_Data.MaxIndices];

  uint32_t offset = 0;

  for (uint32_t i = 0; i < s_Data.MaxIndices; i += 36) {
    // Each face is 4 verts
    for (uint32_t face = 0; face < 6; face++) {
      uint32_t v = offset + face * 4;
      uint32_t idx = i + face * 6;

      cubeIndices[idx + 0] = v + 0;
      cubeIndices[idx + 1] = v + 1;
      cubeIndices[idx + 2] = v + 2;
      cubeIndices[idx + 3] = v + 2;
      cubeIndices[idx + 4] = v + 3;
      cubeIndices[idx + 5] = v + 0;
    }

    offset += 24;
  }

  Ref<IndexBuffer> cubeIndexBuffer =
      IndexBuffer::Create(cubeIndices, s_Data.MaxIndices);
  s_Data.CubeVertexArray->SetIndexBuffer(cubeIndexBuffer);

  s_Data.CubeVertexBufferBase = new QuadVertex[s_Data.MaxVertices];

  // ---- FRONT ----
  s_Data.CubeVertexPositions[0] = {-0.5f, -0.5f, 0.5f, 1.0f};
  s_Data.CubeVertexPositions[1] = {0.5f, -0.5f, 0.5f, 1.0f};
  s_Data.CubeVertexPositions[2] = {0.5f, 0.5f, 0.5f, 1.0f};
  s_Data.CubeVertexPositions[3] = {-0.5f, 0.5f, 0.5f, 1.0f};

  // ---- BACK ----
  s_Data.CubeVertexPositions[4] = {0.5f, -0.5f, -0.5f, 1.0f};
  s_Data.CubeVertexPositions[5] = {-0.5f, -0.5f, -0.5f, 1.0f};
  s_Data.CubeVertexPositions[6] = {-0.5f, 0.5f, -0.5f, 1.0f};
  s_Data.CubeVertexPositions[7] = {0.5f, 0.5f, -0.5f, 1.0f};

  // ---- LEFT ----
  s_Data.CubeVertexPositions[8] = {-0.5f, -0.5f, -0.5f, 1.0f};
  s_Data.CubeVertexPositions[9] = {-0.5f, -0.5f, 0.5f, 1.0f};
  s_Data.CubeVertexPositions[10] = {-0.5f, 0.5f, 0.5f, 1.0f};
  s_Data.CubeVertexPositions[11] = {-0.5f, 0.5f, -0.5f, 1.0f};

  // ---- RIGHT ----
  s_Data.CubeVertexPositions[12] = {0.5f, -0.5f, 0.5f, 1.0f};
  s_Data.CubeVertexPositions[13] = {0.5f, -0.5f, -0.5f, 1.0f};
  s_Data.CubeVertexPositions[14] = {0.5f, 0.5f, -0.5f, 1.0f};
  s_Data.CubeVertexPositions[15] = {0.5f, 0.5f, 0.5f, 1.0f};

  // ---- TOP ----
  s_Data.CubeVertexPositions[16] = {-0.5f, 0.5f, 0.5f, 1.0f};
  s_Data.CubeVertexPositions[17] = {0.5f, 0.5f, 0.5f, 1.0f};
  s_Data.CubeVertexPositions[18] = {0.5f, 0.5f, -0.5f, 1.0f};
  s_Data.CubeVertexPositions[19] = {-0.5f, 0.5f, -0.5f, 1.0f};

  // ---- BOTTOM ----
  s_Data.CubeVertexPositions[20] = {-0.5f, -0.5f, -0.5f, 1.0f};
  s_Data.CubeVertexPositions[21] = {0.5f, -0.5f, -0.5f, 1.0f};
  s_Data.CubeVertexPositions[22] = {0.5f, -0.5f, 0.5f, 1.0f};
  s_Data.CubeVertexPositions[23] = {-0.5f, -0.5f, 0.5f, 1.0f};

  s_Data.Shaders.Load("QuadShader", "Resources/Shader/2D/QuadShader.vert",
                      "Resources/Shader/2D/QuadShader.frag");

  int32_t samplers[s_Data.MaxTextureSlots];
  for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
    samplers[i] = i;
  s_Data.Shaders.Get("QuadShader")
      ->SetIntArray("u_Textures", samplers, s_Data.MaxTextureSlots);

  TextureSpecification spec;
  spec.Format = ImageFormat::RGBA8;
  spec.MinFilter = TextureParameter::Linear;
  spec.MagFilter = TextureParameter::Linear;
  spec.WrapOption = TextureWrapParameter::Repeat;
  s_Data.DefaultTexture = Texture2D::Create(spec);
  uint32_t data = 0xffffffff;
  s_Data.DefaultTexture->SetData(Buffer(&data, sizeof(uint32_t)));

  s_Data.TextureSlots[0] = s_Data.DefaultTexture;
  for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
    s_Data.TextureSlots[i] = s_Data.DefaultTexture;

  s_Data.CameraUniformBuffer =
      UniformBuffer::Create(sizeof(Renderer3DData::CameraBuffer), 0);
}

void Renderer3D::Shutdown() { CE_PROFILE_FUNCTION(); }

void Renderer3D::BeginScene(const EditorCamera &camera) {
  CE_PROFILE_FUNCTION();

  if (m_SceneActive)
    CE_API_ASSERT(false, "No end scene!");

  m_SceneActive = true;

  s_Data.CameraBuffer.ViewProjection = camera.GetViewProjection();
  s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer,
                                      sizeof(Renderer3DData::CameraBuffer));

  StartBatch();
}

void Renderer3D::EndScene() {
  CE_PROFILE_FUNCTION();

  if (!m_SceneActive)
    CE_API_ASSERT(false, "No begin scene!");

  m_SceneActive = false;
  Flush();
}

void Renderer3D::StartBatch() {
  s_Data.CubeIndexCount = 0;
  s_Data.CubeVertexBufferPtr = s_Data.CubeVertexBufferBase;

  s_Data.TextureSlotIndex = 1;
}

void Renderer3D::Flush() {
  if (s_Data.CubeIndexCount) {
    uint32_t dataSize = (uint32_t)((uint8_t *)s_Data.CubeVertexBufferPtr -
                                   (uint8_t *)s_Data.CubeVertexBufferBase);
    s_Data.CubeVertexBuffer->SetData(s_Data.CubeVertexBufferBase, dataSize);

    for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
      s_Data.TextureSlots[i]->Bind(i);

    s_Data.Shaders.Get("QuadShader")->Bind();
    RenderCommand::DrawIndexed(s_Data.CubeVertexArray, s_Data.CubeIndexCount);
    // s_Data.Stats.DrawCalls++;
  }
}

void Renderer3D::NextBatch() {
  Flush();
  StartBatch();
}

void Renderer3D::DrawCube(const glm::mat4 &transform,
                          SpriteRendererComponent &sprite, uint32_t entityID) {
  CE_PROFILE_FUNCTION();
  if (!m_SceneActive)
    CE_API_ASSERT(false, "No begin scene!");
  float textureIndex = 0.0f;
  constexpr glm::vec2 textureCoords[4] = {
      {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};

  Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(sprite.Texture);
  if (texture) {

    for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++) {

      if (*s_Data.TextureSlots[i] == *texture.get()) {
        textureIndex = (float)i;
        break;
      }
    }

    if (textureIndex == 0.0f) {
      textureIndex = (float)s_Data.TextureSlotIndex;
      s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
      s_Data.TextureSlotIndex++;
    }
  }

  if (s_Data.CubeIndexCount >= s_Data.MaxIndices)
    NextBatch();

  for (size_t i = 0; i < 24; i++) {
    s_Data.CubeVertexBufferPtr->Position =
        transform * s_Data.CubeVertexPositions[i];
    s_Data.CubeVertexBufferPtr->Color = sprite.Color;
    s_Data.CubeVertexBufferPtr->TexCoord = textureCoords[i % 4];
    s_Data.CubeVertexBufferPtr->TexIndex = textureIndex;
    s_Data.CubeVertexBufferPtr->TilingFactor = sprite.TilingFactor;
    s_Data.CubeVertexBufferPtr->EntityID = entityID;

    s_Data.CubeVertexBufferPtr++;
  }

  s_Data.CubeIndexCount += 36;
  // s_Data.Stats.QuadCount++;
}

} // namespace CatEngine
