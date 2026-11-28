#pragma once

#include "UITypes.h"

#include <glm/glm.hpp>

namespace CatEngine {

struct SelectableComponent {
  bool Interactable = true;
  bool Navigable = true;     // reachable by Tab / arrow keys
  bool Draggable = false;    // receives BeginDrag/Drag/EndDrag (sliders always do)
  UITransition Transition = UITransition::ColorTint;
  glm::vec4 NormalTint{1.0f};
  glm::vec4 HighlightedTint{0.96f, 0.96f, 0.96f, 1.0f};
  glm::vec4 PressedTint{0.78f, 0.78f, 0.78f, 1.0f};
  glm::vec4 SelectedTint{0.96f, 0.96f, 0.96f, 1.0f};
  glm::vec4 DisabledTint{0.78f, 0.78f, 0.78f, 0.5f};
  float FadeDuration = 0.1f;    // seconds, 0 = instant
  bool TintChildren = false;    // also tint descendant graphics (e.g. a button's icon)
  bool EditDirty = false;
  bool PrefabDirty = false;
};

}
