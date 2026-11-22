#pragma once

#include <filesystem>
namespace CatEngine {

// Will rely on the AssetBrowserPanel for the Scene Instance
class AssetPostprocessor {
public:
  static void OnFileModified(const std::filesystem::path &filePath);
private:
  static void ReloadPrefabAsset(const std::filesystem::path &filePath);
  static void ReloadSceneAsset(const std::filesystem::path &filePath);
  static void ReloadTextureAsset(const std::filesystem::path &filePath);
};

}
