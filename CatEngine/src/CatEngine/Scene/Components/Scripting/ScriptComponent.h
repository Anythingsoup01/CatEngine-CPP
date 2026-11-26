#pragma once

#include "CatEngine/Core/UUID.h"
#include <string>

namespace CatEngine {

struct ScriptComponent {
  bool Active = true;
  UUID ScriptID;
  std::string ClassName;
  bool Loaded = false;

  bool RuntimeDirty = false;
  bool EditDirty = false;
  bool PrefabDirty = false;

  ScriptComponent(const UUID &id, const std::string &name)
    : ScriptID(id), ClassName(name) {}

  ScriptComponent() = default;
  ScriptComponent(const ScriptComponent &) = default;
  ~ScriptComponent() = default;
};
} // namespace CatEngine
