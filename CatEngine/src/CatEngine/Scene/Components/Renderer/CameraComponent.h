#pragma once

#include "CatEngine/Scene/SceneCamera.h" // adjust to your path

#include <glm/glm.hpp>

namespace CatEngine {

enum class CameraAspectMode : uint8_t {
  MatchViewport = 0, // fill the panel/window, aspect follows it
  Ratio,             // fixed W:H ratio, letterboxed
  Pixels,            // fixed output resolution (e.g. 1920x1080), letterboxed
};
enum class CameraClearMode : uint8_t { SolidColor = 0, DontClear };
enum class CameraSizeMode : uint8_t { Units = 0, PixelsPerUnit }; // orthographic only

struct CameraViewInfo {
  glm::ivec2 Origin{0}; // bottom-left, in framebuffer pixels
  glm::ivec2 Size{1};
  float Aspect = 1.0f;
};

struct CameraComponent {
  bool Active = true;
  SceneCamera Camera;
  bool Primary = true;

  // Output shape
  CameraAspectMode AspectMode = CameraAspectMode::MatchViewport;
  glm::vec2 AspectRatio{16.0f, 9.0f};
  glm::vec2 Resolution{1920.0f, 1080.0f};
  bool IntegerScaling = false;              // Pixels mode: only whole-number upscales
  glm::vec4 ViewportRect{0.0f, 0.0f, 1.0f, 1.0f}; // x, y, w, h, normalized (minimaps, split screen)

  // Orthographic sizing
  CameraSizeMode OrthoSizeMode = CameraSizeMode::Units;
  float PixelsPerUnit = 100.0f;

  // Background
  CameraClearMode ClearMode = CameraClearMode::SolidColor;
  glm::vec4 BackgroundColor{0.1f, 0.1f, 0.1f, 1.0f};

  bool RuntimeDirty = false;
  bool EditDirty = false;
  bool PrefabDirty = false;

  CameraComponent() = default;
  CameraComponent(const CameraComponent &) = default;
  ~CameraComponent() = default;

  bool IsFixedAspect() const { return AspectMode != CameraAspectMode::MatchViewport; }

  // Where this camera renders inside a framebuffer of the given size.
  CameraViewInfo Resolve(uint32_t fbW, uint32_t fbH) const {
    const glm::vec4 r = glm::clamp(ViewportRect, glm::vec4(0.0f), glm::vec4(1.0f));
    const glm::vec2 fb{(float)std::max(fbW, 1u), (float)std::max(fbH, 1u)};
    const glm::vec2 subOrigin{r.x * fb.x, r.y * fb.y};
    const glm::vec2 subSize{std::max(r.z, 0.001f) * fb.x, std::max(r.w, 0.001f) * fb.y};
    const float subAspect = subSize.x / subSize.y;

    const glm::vec2 res = glm::max(Resolution, glm::vec2(1.0f));
    float target = subAspect;
    if (AspectMode == CameraAspectMode::Ratio && AspectRatio.x > 0.0f && AspectRatio.y > 0.0f)
      target = AspectRatio.x / AspectRatio.y;
    else if (AspectMode == CameraAspectMode::Pixels)
      target = res.x / res.y;

    glm::vec2 size = subSize;
    if (IsFixedAspect()) {
      size = target > subAspect ? glm::vec2(subSize.x, subSize.x / target)
                                : glm::vec2(subSize.y * target, subSize.y);
      if (AspectMode == CameraAspectMode::Pixels && IntegerScaling) {
        const float s = std::floor(std::min(subSize.x / res.x, subSize.y / res.y));
        if (s >= 1.0f)
          size = res * s;
      }
    }

    const glm::vec2 origin = subOrigin + (subSize - size) * 0.5f;
    CameraViewInfo info;
    info.Size = glm::max(glm::ivec2(glm::round(size)), glm::ivec2(1));
    info.Origin = glm::ivec2(glm::round(origin));
    info.Aspect = IsFixedAspect() ? target : subAspect;
    return info;
  }

  // Pushes the resolved shape into the projection.
  void ApplyView(const CameraViewInfo &v) {
    Camera.SetAspectRatio(v.Aspect);
    if (OrthoSizeMode == CameraSizeMode::PixelsPerUnit && PixelsPerUnit > 0.0f &&
        Camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic) {
      // Pixels mode uses the design resolution, otherwise the actual output height.
      const float refH = AspectMode == CameraAspectMode::Pixels ? std::max(Resolution.y, 1.0f)
                                                                : (float)v.Size.y;
      const float size = refH / PixelsPerUnit; // size = full vertical extent (Hazel convention)
      if (std::abs(size - Camera.GetOrthographicSize()) > 1e-5f)
        Camera.SetOrthographicSize(size);
    }
  }
};

} // namespace CatEngine
