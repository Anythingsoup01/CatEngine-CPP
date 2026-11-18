#include "Commands.h"
#include "CatScriptCore/Scene/Editor/Console.h"

namespace CatRuntime {

Object Scene::Instantiate(const Prefab &prefab) {
  if (prefab.GetAssetID() == 0) {
    Console::Error("Instantiate Error: Prefab Asset ID is 0");
    return Object(0);
  }

  uint64_t out = 0;
  Scene_InstantiatePrefab(prefab.GetAssetID(), &out);
  if (out == 0) {
    Console::Error("Instantiate Error: Failed to get Prefab with ID: %lu", prefab.GetAssetID());
    return Object(0);
  }

  return Object(out);
}

void Scene::Destroy(const Object &object) {
  if (!object) {
    Console::Error("Destroy Error: Object doesn't exist");
    return;
  }

  Scene_DestroyObject(object);
}

}
