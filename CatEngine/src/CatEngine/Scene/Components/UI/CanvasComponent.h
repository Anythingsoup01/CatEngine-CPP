#pragma once

#include <glm/glm.hpp>

namespace CatEngine {

enum class CanvasRenderMode : uint8_t {
  ScreenSpaceOverlay = 0, // Drawn over everything, fills the viewport
  // ScreenSpaceCamera, WorldSpace: add later
};

enum class CanvasScaleMode : uint8_t {
  ConstantPixelSize = 0, // ScaleFactor is used as-is
  ScaleWithScreenSize,   // Scales relative to ReferenceResolution
};

enum class CanvasScreenMatchMode : uint8_t {
  MatchWidthOrHeight = 0, // Blend between width and height using MatchWidthOrHeight
  Expand,                 // Whole reference area always fits inside the screen
  Shrink,                 // Reference area always covers the whole screen
};

struct CanvasComponent {
  CanvasRenderMode RenderMode = CanvasRenderMode::ScreenSpaceOverlay;
  int32_t SortOrder = 0; // Higher draws on top of lower

  CanvasScaleMode ScaleMode = CanvasScaleMode::ScaleWithScreenSize;
  float ScaleFactor = 1.0f; // Used by ConstantPixelSize
  glm::vec2 ReferenceResolution{1920.0f, 1080.0f};
  CanvasScreenMatchMode ScreenMatchMode = CanvasScreenMatchMode::MatchWidthOrHeight;
  float MatchWidthOrHeight = 0.5f; // 0 = match width, 1 = match height

  bool EditDirty = false;
  bool PrefabDirty = false;

  // Returns the multiplier applied to RectTransform Position/Size for the given viewport.
  float ComputeScaleFactor(const glm::vec2 &viewport) const {
    if (viewport.x <= 0.0f || viewport.y <= 0.0f)
      return 1.0f;

    switch (ScaleMode) {
    case CanvasScaleMode::ConstantPixelSize:
      return std::max(ScaleFactor, 0.0001f);

    case CanvasScaleMode::ScaleWithScreenSize: {
      const glm::vec2 ref = glm::max(ReferenceResolution, glm::vec2{1.0f});
      const float scaleX = viewport.x / ref.x;
      const float scaleY = viewport.y / ref.y;

      switch (ScreenMatchMode) {
      case CanvasScreenMatchMode::Expand:
        return std::min(scaleX, scaleY);
      case CanvasScreenMatchMode::Shrink:
        return std::max(scaleX, scaleY);
      case CanvasScreenMatchMode::MatchWidthOrHeight:
      default: {
        // Blend in log space so 2x wider and 2x narrower cancel out evenly (same as Unity).
        const float t = std::clamp(MatchWidthOrHeight, 0.0f, 1.0f);
        const float logW = std::log2(scaleX);
        const float logH = std::log2(scaleY);
        return std::exp2(logW + (logH - logW) * t);
      }
      }
    }
    }
    return 1.0f;
  }
};

} // namespace CatEngine
