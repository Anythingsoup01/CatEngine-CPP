#include "cepch.h" // IWYU pragma: keep
#include "PrefabImporter.h"

#include "CatEngine/Project/Project.h"
#include "CatEngine/Prefab/PrefabSerializer.h"

namespace CatEngine {

Ref<Prefab> PrefabImporter::ImportPrefab(AssetHandle handle, Asset::MetaData &metaData) {
  CE_PROFILE_FUNCTION();
  return LoadPrefab(handle, Project::GetAssetDirectory() / metaData.SourceFilePath);
}
Ref<Prefab> PrefabImporter::ConstImportPrefab(AssetHandle handle, const Asset::MetaData &metaData) {
  CE_PROFILE_FUNCTION();
  return LoadPrefab(handle, Project::GetAssetDirectory() / metaData.SourceFilePath);
}
Ref<Prefab> PrefabImporter::LoadPrefab(const AssetHandle &handle, const std::filesystem::path &filePath) {
  Ref<Prefab> prefab = CreateRef<Prefab>();
  prefab->m_Handle = handle;
  PrefabSerializer serializer(prefab);
  serializer.Deserialize(filePath);

  return prefab;
}
} // namespace CatEngine
