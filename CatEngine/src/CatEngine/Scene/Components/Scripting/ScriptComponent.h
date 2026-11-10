#pragma once

#include "CatEngine/Core/UUID.h"
#include <string>

namespace CatEngine {

struct ScriptComponent {
  UUID ScriptID;
  std::string ClassName;
  std::string MangledName;
  int SelectedScript;
  bool Loaded = false;

  ScriptComponent(const UUID &id, const std::string &name)
    : ScriptID(id), ClassName(name), SelectedScript(0) {}

  ScriptComponent() = default;
  ScriptComponent(const ScriptComponent &) = default;
  ~ScriptComponent() = default;
};
} // namespace CatEngine
