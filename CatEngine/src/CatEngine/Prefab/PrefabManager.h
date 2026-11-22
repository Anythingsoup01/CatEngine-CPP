#pragma once

#include "CatEngine/AssetManager/Asset.h"

namespace CatEngine {

class Entity;

class PrefabManager {
public:
  PrefabManager() = default;

  const std::vector<AssetHandle> &GetPrefabs() const { return m_Prefabs; }

  void CreateNewPrefab(const std::filesystem::path &targetDir);
  void CreateNewPrefabFromEntity(Entity e, const std::filesystem::path &targetDir);

  void AddPrefab(const AssetHandle &handle);
  void RemovePrefab(const AssetHandle &handle);

  static PrefabManager &Get() { static PrefabManager s_Manager; return s_Manager; }
private:
  std::vector<AssetHandle> m_Prefabs;
};

}
