#pragma once

#include "CatEngine/Core/UUID.h"
#include <string>

namespace CatEngine {

struct ScriptComponent {
  UUID ScriptID;
  std::string ClassName;
  int SelectedScript = 0;
  bool Loaded = false;

  ScriptComponent() = default;
  ScriptComponent(const ScriptComponent &) = default;
  ~ScriptComponent() = default;
};
} // namespace CatEngine
