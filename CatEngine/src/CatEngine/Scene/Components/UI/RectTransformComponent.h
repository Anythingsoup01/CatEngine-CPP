#pragma once

#include "UITypes.h"

namespace CatEngine {

struct RectTransformComponent {
  UIAnchorPreset AnchorPreset = UIAnchorPreset::Center;
  glm::vec2 AnchorMin{0.5f, 0.5f}; // Normalized [0, 1] inside parent rect
  glm::vec2 AnchorMax{0.5f, 0.5f};
  glm::vec2 Pivot{0.5f, 0.5f};     // Normalized [0, 1] around own bounds
  glm::vec2 Position{0.0f, 0.0f};  // Offset in unscaled canvas units
  glm::vec2 Size{160.0f, 48.0f};   // Width/height when AnchorMin == AnchorMax
  float Rotation = 0.0f;           // Degrees (for future rotation support)
  bool EditDirty = false;
  bool PrefabDirty = false;

  void ApplyAnchorPreset(const UIAnchorPreset &preset) {
    AnchorPreset = preset;
    switch (preset) {
      case UIAnchorPreset::TopLeft:
        AnchorMin = {0.0f, 1.0f};
        AnchorMax = {0.0f, 1.0f};
        break;
      case UIAnchorPreset::TopCenter:
        AnchorMin = {0.5f, 1.0f};
        AnchorMax = {0.5f, 1.0f};
        break;
      case UIAnchorPreset::TopRight:
        AnchorMin = {1.0f, 1.0f};
        AnchorMax = {1.0f, 1.0f};
        break;
      case UIAnchorPreset::MiddleLeft:
        AnchorMin = {0.0f, 0.5f};
        AnchorMax = {0.0f, 0.5f};
        break;
      case UIAnchorPreset::Center:
        AnchorMin = {0.5f, 0.5f};
        AnchorMax = {0.5f, 0.5f};
        break;
      case UIAnchorPreset::MiddleRight:
        AnchorMin = {1.0f, 0.5f};
        AnchorMax = {1.0f, 0.5f};
        break;
      case UIAnchorPreset::BottomLeft:
        AnchorMin = {0.0f, 0.0f};
        AnchorMax = {0.0f, 0.0f};
        break;
      case UIAnchorPreset::BottomCenter:
        AnchorMin = {0.5f, 0.0f};
        AnchorMax = {0.5f, 0.0f};
        break;
      case UIAnchorPreset::BottomRight:
        AnchorMin = {1.0f, 0.0f};
        AnchorMax = {1.0f, 0.0f};
        break;
      case UIAnchorPreset::StretchAll:
        AnchorMin = {0.0f, 0.0f};
        AnchorMax = {1.0f, 1.0f};
        break;
      case UIAnchorPreset::Custom:
        break;
    }
  }
};

}
