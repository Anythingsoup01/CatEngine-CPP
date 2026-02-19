#pragma once

#include "Camera.h"
#include "EditorCamera.h"

#include <glm/glm.hpp>

#include "CatEngine/Scene/Components/2D/CircleRenderer.h"
#include "CatEngine/Scene/Components/2D/SpriteRenderer.h"

namespace CatEngine {
class Renderer2D {
public:
  static void Init();
  static void Shutdown();

  static void BeginScene(const Camera &camera, const glm::mat4 &transform);
  static void BeginScene(const EditorCamera &camera);
  static void EndScene();

  static void StartBatch();
  static void Flush();

  static void NextBatch();

  static void SetLineThickness(float thickness);

  static void DrawSprite(const glm::mat4 &transform,
                         SpriteRendererComponent &sprite, int entityID);
  static void DrawCircle(const glm::mat4 &transform,
                         CircleRendererComponent &crc, int entityID);
  static void DrawLine(const glm::vec3 &p0, const glm::vec3 &p1,
                       const glm::vec4 &color, int entityID);
  static void DrawRect(const glm::mat4 &transform, const glm::vec4 &color,
                       int entityID = -1);

  struct Statistics {
    uint32_t DrawCalls = 0;
    uint32_t QuadCount = 0;

    uint32_t GetTotalVertexCount() const { return QuadCount * 4; }
    uint32_t GetTotalIndexCount() const { return QuadCount * 6; }
  };

  static void ResetStats();
  static Statistics GetStats();

private:
  static inline bool m_SceneActive = false;
};
}; // namespace CatEngine
