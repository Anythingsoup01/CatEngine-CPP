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

  std::unordered_map<UUID, std::vector<std::string>> GetDirtyEntities(const AssetHandle &prefabID, Entity rootEntity);

  static PrefabManager &Get() { static PrefabManager s_Manager; return s_Manager; }
private:
  void GetDirtyComponents(Entity e,  std::unordered_map<UUID, std::vector<std::string>> *changeMap);

  void ApplyEntityOverridesRecursive(Entity instanceEntity, Entity prefabEntity, bool isRoot);
  void CopyDirtyComponentsToPrefab(Entity srcInstanceEntity, Entity dstPrefabEntity, bool isRoot);
  void ClearInstanceOverrideFlagsRecursive(Entity current);
private:
  std::vector<AssetHandle> m_Prefabs;
};

}
