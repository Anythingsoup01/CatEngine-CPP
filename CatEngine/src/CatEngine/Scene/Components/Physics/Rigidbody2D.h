#pragma once

namespace CatEngine {
struct Rigidbody2DComponent {
  bool Active = true;
  enum class BodyType { Static = 0, Kinematic, Dynamic };

  BodyType Type = BodyType::Static;
  bool FixedRotation = false;

  bool RuntimeDirty = false;
  bool EditDirty = false;
  bool PrefabDirty = false;

  // Storage for runtime
  void *RuntimeBody = nullptr;

  Rigidbody2DComponent() = default;
  Rigidbody2DComponent(const Rigidbody2DComponent &) = default;
  ~Rigidbody2DComponent() = default;
};
} // namespace CatEngine
