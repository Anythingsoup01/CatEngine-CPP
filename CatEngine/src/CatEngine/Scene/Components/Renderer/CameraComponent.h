#pragma once

#include "CatEngine/Scene/SceneCamera.h"

namespace CatEngine {

struct CameraComponent {
  bool Active = true;
  SceneCamera Camera;
  bool Primary = true;
  bool FixedAspectRatio = false;

  bool RuntimeDirty = false;
  bool EditDirty = false;
  bool PrefabDirty = false;


  CameraComponent() = default;
  CameraComponent(const CameraComponent &) = default;
  ~CameraComponent() = default;
};
} // namespace CatEngine
