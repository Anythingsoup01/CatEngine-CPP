#pragma once

#include "ScriptComponent.h"


namespace CatEngine {

struct ScriptComponentContainer {
  std::vector<ScriptComponent> Scripts;
  std::vector<std::string> LoadedScriptNames;
};

}
