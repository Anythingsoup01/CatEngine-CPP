#pragma once

#include "CatEngine/AssetManager/Asset.h"
#include "CatEngine/AssetManager/AssetManagerBase.h"
#include "CatEngine/AssetManager/EditorAssetManager.h"
#include "CatEngine/AssetManager/RuntimeAssetManager.h"

namespace CatEngine {
struct ProjectConfig {
  std::string Name = "Untitled";
  std::vector<AssetHandle> OpenScenes;
  std::filesystem::path AssetDirectory;
  std::filesystem::path AssetRegistryPath; // Relative to the asset directory
};

class Project {
public:
  Project();

  static const std::filesystem::path &GetProjectDirectory() {
    CE_API_ASSERT(s_ActiveProject, "No Active Project");
    return s_ActiveProject->m_ActiveProjectDirectory;
  }

  static std::filesystem::path GetAssetDirectory() {
    CE_API_ASSERT(s_ActiveProject, "No Active Project!");
    return GetProjectDirectory() / s_ActiveProject->m_Config.AssetDirectory;
  }

  static ProjectConfig &GetConfig() {
    CE_API_ASSERT(s_ActiveProject, "No Active Project");
    return s_ActiveProject->m_Config;
  }

  static std::filesystem::path
  GetAssetFileSystemPath(const std::filesystem::path &path) {
    CE_API_ASSERT(s_ActiveProject, "No Active Project");
    return GetProjectDirectory() / GetAssetDirectory() / path;
  }

  static void OpenScene(AssetHandle sceneHandle) {
    CE_API_ASSERT(s_ActiveProject, "No Active Project");
    auto &scenes = s_ActiveProject->m_Config.OpenScenes;
    for (auto it = scenes.begin(); it != scenes.end(); it++) {
      auto handle = *it;
      // If a scene gets reopened we should move it to the top of
      // the stack
      if (sceneHandle == handle) {
        scenes.erase(it);
        break;
      }
    }
    s_ActiveProject->m_Config.OpenScenes.push_back(sceneHandle);
  }

  // With this scene layout, users can only close the current scene
  // so all we have to do is pop the back
  static void CloseScene() {
    CE_API_ASSERT(s_ActiveProject, "No Active Project");
    auto &scenes = s_ActiveProject->m_Config.OpenScenes;
    if (scenes.size() > 1) // At least one scene is required to be open
      scenes.pop_back();
  }

  static std::vector<AssetHandle> &GetOpenScenes() {
    CE_API_ASSERT(s_ActiveProject, "No Active Project");
    return s_ActiveProject->m_Config.OpenScenes;
  }

  static Ref<Project> &GetActive() { return s_ActiveProject; }

  Ref<AssetManagerBase> GetAssetManager() { return m_AssetManager; }
  Ref<RuntimeAssetManager> GetRuntimeAssetManager() {
    return std::static_pointer_cast<RuntimeAssetManager>(m_AssetManager);
  }
  Ref<EditorAssetManager> GetEditorAssetManager() {
    return std::static_pointer_cast<EditorAssetManager>(m_AssetManager);
  }

  static Ref<Project> New();
  static Ref<Project> Load(const std::filesystem::path &filePath); // TODO: Possibly make a bool
  static bool Save(const std::filesystem::path &filePath);

  static void UpdateAssetManager();

private:
  ProjectConfig m_Config;
  std::filesystem::path m_ActiveProjectDirectory;
  inline static Ref<Project> s_ActiveProject = CreateRef<Project>();

  Ref<AssetManagerBase> m_AssetManager;
};
} // namespace CatEngine
