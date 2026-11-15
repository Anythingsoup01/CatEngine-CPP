#include "cepch.h"
#include "PrefabImporter.h"

#include "CatEngine/Project/Project.h"
#include "CatEngine/Scene/PrefabSerializer.h"

namespace CatEngine {

Ref<Prefab> PrefabImporter::ImportPrefab(AssetHandle handle,
                                         Asset::MetaData &metaData) {
  CE_PROFILE_FUNCTION();
  return LoadPrefab(Project::GetAssetDirectory() / metaData.SourceFilePath);
}
Ref<Prefab> PrefabImporter::ConstImportPrefab(AssetHandle handle,
                                         const Asset::MetaData &metaData) {
  CE_PROFILE_FUNCTION();
  return LoadPrefab(Project::GetAssetDirectory() / metaData.SourceFilePath);
}
Ref<Prefab> PrefabImporter::LoadPrefab(const std::filesystem::path &filePath) {
  Ref<Prefab> prefab = CreateRef<Prefab>();
  PrefabSerializer serializer(prefab);
  serializer.Deserialize(filePath);

  return prefab;
}
} // namespace CatEngine
