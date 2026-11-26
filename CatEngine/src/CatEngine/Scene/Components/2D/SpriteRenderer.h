#pragma once

#include "CatEngine/Core/Core.h"
#include "CatEngine/Renderer/Texture.h"

#include <glm/glm.hpp>

namespace CatEngine {

struct SpriteRendererComponent {
  bool Active = true;
  glm::vec4 Color{1.f, 1.f, 1.f, 1.f};
  AssetHandle Texture = 0;
  float TilingFactor = 1.0f;

  bool RuntimeDirty = false;
  bool EditDirty = false;
  bool PrefabDirty = false;

  SpriteRendererComponent() = default;
  SpriteRendererComponent(const SpriteRendererComponent &) = default;
  SpriteRendererComponent(const glm::vec4 &color, AssetHandle texture)
      : Color(color), Texture(texture) {}

  ~SpriteRendererComponent() = default;
};
} // namespace CatEngine
