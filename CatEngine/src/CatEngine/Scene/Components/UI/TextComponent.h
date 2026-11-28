#pragma once

#include "UITypes.h"

#include "CatEngine/AssetManager/Asset.h"

#include <glm/glm.hpp>

namespace CatEngine {

struct UITextComponent {
  std::string Text = "New Text";
  AssetHandle Font = 0; // 0 = default font
  float FontSize = 24.0f; // canvas units, scaled by the canvas
  glm::vec4 Color{0.2f, 0.2f, 0.2f, 1.0f};
  bool EditDirty = false;
  bool PrefabDirty = false;
};

}
