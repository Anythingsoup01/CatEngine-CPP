#pragma once

#include "CatEngine/AssetManager/Asset.h"
#include "CatEngine/Scene/Scene.h"

namespace CatEngine {

class Prefab : public Asset {
public:
  Prefab() = default;
  ~Prefab() = default;

  Ref<Scene> GetScene() { return m_PrefabScene; }

  static AssetType GetStaticType() { return AssetType::Prefab; }
  AssetType GetType() const override { return GetStaticType(); }
private:
  Ref<Scene> m_PrefabScene = CreateRef<Scene>();
  friend class PrefabSerializer;
};

}
