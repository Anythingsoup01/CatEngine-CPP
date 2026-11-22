#pragma once

#include "CatEngine/AssetManager/Asset.h"

#include "CatEngine/Prefab/Prefab.h"

namespace CatEngine {
class PrefabImporter {
public:
  // This version let's us import a texture and modify the meta data to properly
  // fit the texture
  static Ref<Prefab> ImportPrefab(AssetHandle handle, Asset::MetaData &metaData);

  // This version is used by the EditorAssetManager when loading meta data from
  // a file
  static Ref<Prefab> ConstImportPrefab(AssetHandle handle, const Asset::MetaData &metaData);

  static Ref<Prefab> LoadPrefab(const AssetHandle &handle, const std::filesystem::path &filePath);
};

} // namespace CatEngine
