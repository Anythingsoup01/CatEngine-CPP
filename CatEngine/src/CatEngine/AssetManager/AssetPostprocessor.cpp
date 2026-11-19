#include "AssetPostprocessor.h"
#include "CatEngine/Editor/PanelManager.h"
#include "CatEngine/Editor/Panels/AssetBrowserPanel.h"
#include "CatEngine/Project/Project.h"
#include <filesystem>

namespace CatEngine {

void AssetPostprocessor::OnFileModified(const std::filesystem::path &filePath) {
  if (filePath.extension() == ".catprefab") {
    ReloadPrefabAsset(filePath);
  }
}

void AssetPostprocessor::ReloadPrefabAsset(const std::filesystem::path &filePath) {
  auto assetManager = Project::GetActive()->GetEditorAssetManager();
  std::filesystem::path assetPath = filePath;
  assetPath.replace_extension(".catprefabasset");
  UUID prefabID = assetManager->GetAssetIDFromPath(assetPath);
  if (prefabID == 0)
    return;

  if (!assetManager->ReloadAssetFromDisk(assetPath))
    return;

  auto assetBrowser = PanelManager::Get().GetPanel<AssetBrowserPanel>("AssetBrowserPanel");
  assetBrowser->GetSceneContext()->OnPrefabReloaded(prefabID);

}

} // namespace CatEngine
