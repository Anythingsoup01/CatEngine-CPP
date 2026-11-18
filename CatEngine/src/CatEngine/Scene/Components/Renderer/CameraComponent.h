#pragma once

#include "CatEngine/Scene/SceneCamera.h"

namespace CatEngine {

struct CameraComponent {
  bool Active = true;
  SceneCamera Camera;
  bool Primary = true;
  bool FixedAspectRatio = false;

  CameraComponent() = default;
  CameraComponent(const CameraComponent &) = default;
  ~CameraComponent() = default;
};
} // namespace CatEngine
