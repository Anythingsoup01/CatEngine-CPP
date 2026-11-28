#pragma once

#include "CatEngine/AssetManager/Asset.h"

#include <glm/glm.hpp>

namespace CatEngine {

struct PanelComponent {
  glm::vec4 Color = {1, 1, 1, 0.9f};
  AssetHandle Texture = 0;
  bool RaycastTarget = true;
  bool EditDirty = false;
  bool PrefabDirty = false;
};

}
