#pragma once

#include "Camera.h"
#include "EditorCamera.h"

#include <glm/glm.hpp>

#include "CatEngine/Scene/Components/2D/SpriteRenderer.h"

namespace CatEngine {

class Renderer3D {
public:
  static void Init();
  static void Shutdown();

  static void BeginScene(const Camera &camera, const glm::mat4 &transform);
  static void BeginScene(const EditorCamera &camera);
  static void EndScene();

  static void StartBatch();
  static void Flush();
  static void NextBatch();

  static void DrawCube(const glm::mat4 &transform,
                       SpriteRendererComponent &sprite, uint32_t entityID);

private:
  static inline bool m_SceneActive = false;
};

} // namespace CatEngine
