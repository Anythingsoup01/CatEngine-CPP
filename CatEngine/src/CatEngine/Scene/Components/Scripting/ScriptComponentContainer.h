#pragma once

#include "ScriptComponent.h"


namespace CatEngine {

struct ScriptComponentContainer {
  std::vector<ScriptComponent> Scripts;
  std::vector<std::string> LoadedScriptNames;

  bool RuntimeDirty = false;
  bool EditDirty = false;
  bool PrefabDirty = false;
};

}
