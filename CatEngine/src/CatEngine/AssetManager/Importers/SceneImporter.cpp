#include "cepch.h" // IWYU pragma: keep
#include "SceneImporter.h"

#include "CatEngine/Project/Project.h"
#include "CatEngine/Scene/SceneSerializer.h"

namespace CatEngine {
Ref<Scene> SceneImporter::ImportScene(AssetHandle handle,
                                      Asset::MetaData &metaData) {
  CE_PROFILE_FUNCTION();
  return LoadScene(Project::GetAssetDirectory() / metaData.SourceFilePath);
}

Ref<Scene> SceneImporter::ConstImportScene(AssetHandle handle,
                                           const Asset::MetaData &metaData) {
  CE_PROFILE_FUNCTION();
  return LoadScene(Project::GetAssetDirectory() / metaData.SourceFilePath);
}

Ref<Scene> SceneImporter::LoadScene(const std::filesystem::path &filePath) {
  Ref<Scene> scene = CreateRef<Scene>();
  SceneSerializer serializer(scene);
  serializer.Deserialize(filePath);

  return scene;
}

} // namespace CatEngine
