#pragma once

namespace CatEngine {

struct CanvasGroupComponent {
  float Alpha = 1.0f;
  bool Interactable = true;
  bool BlocksRaycasts = true;
  bool IgnoreParentGroups = false;
  bool EditDirty = false;
  bool PrefabDirty = false;
};

}
