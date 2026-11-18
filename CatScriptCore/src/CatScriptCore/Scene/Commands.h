#pragma once

#include "CatScriptCore/Asset/Prefab.h"
#include "CatScriptCore/Scene/Component/Object.h"

namespace CatRuntime {

class Scene {
public:
  static Object Instantiate(const Prefab &prefab);
  static void Destroy(const Object &object);
};

}
