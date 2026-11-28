#pragma once

#include "CatEngine/AssetManager/Asset.h"

#include <glm/glm.hpp>

namespace CatEngine {

struct ButtonVisualComponent {
  glm::vec4 NormalColor = {1, 1, 1, 1};
  AssetHandle Texture = 0;
  bool RaycastTarget = true;
  bool EditDirty = false;
  bool PrefabDirty = false;
};

}
