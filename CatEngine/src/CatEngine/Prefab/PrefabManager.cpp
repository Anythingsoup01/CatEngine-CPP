#include "cepch.h"
#include "PrefabManager.h"

#include "CatEngine/Scene/Entity.h"
#include "PrefabSerializer.h"

#include "CatEngine/Project/Project.h"

namespace CatEngine {

void PrefabManager::CreateNewPrefab(const std::filesystem::path &targetDirectory) {
  std::filesystem::path prefabPath =  targetDirectory / "NewPrefab.catprefab";

  int counter = 0;
  while (std::filesystem::exists(prefabPath)) {
    prefabPath =
      targetDirectory / (prefabPath.stem().generic_string() + "_" + std::to_string(++counter) + prefabPath.extension().generic_string());
  }

  Ref<Prefab> newPrefab = CreateRef<Prefab>();
  auto scene = newPrefab->GetScene();
  scene->CreateEntity("New Entity");
  PrefabSerializer serializer(newPrefab);
  serializer.Serialize(prefabPath);

  std::filesystem::path relativePath = std::filesystem::relative(prefabPath, Project::GetAssetDirectory());
  Project::GetActive()->GetEditorAssetManager()->ImportNewAsset(relativePath);

  AddPrefab(newPrefab->GetAssetHandle());

}

void PrefabManager::CreateNewPrefabFromEntity(Entity e, const std::filesystem::path &targetDirectory) {
  if (!e)
    return;

  std::string entityName = e.GetName();
  std::filesystem::path prefabPath = targetDirectory / (entityName + ".catprefab");

  int counter = 0;
  while (std::filesystem::exists(prefabPath)) {
    prefabPath =
      targetDirectory /
      (entityName + "_" + std::to_string(++counter) + ".catprefab");
  }

  Ref<Prefab> newPrefab = CreateRef<Prefab>();
  PrefabSerializer serializer(newPrefab);
  serializer.Serialize(prefabPath, e);

  std::filesystem::path relativePath = std::filesystem::relative(prefabPath, Project::GetAssetDirectory());
  Project::GetActive()->GetEditorAssetManager()->ImportNewAsset(relativePath);

  AddPrefab(newPrefab->GetAssetHandle());
}

void PrefabManager::AddPrefab(const AssetHandle &handle) {
  m_Prefabs.push_back(handle);
}

void PrefabManager::RemovePrefab(const AssetHandle &handle) {
  for (auto it = m_Prefabs.begin(); it != m_Prefabs.end(); it++) {
    if (handle == *it) {
      m_Prefabs.erase(it);
      break;
    }
  }
}

} // namespace CatEngine
