#pragma once

#include "CatEngine/Renderer/Texture.h"

#include "CatEngine/Editor/EditorPanel.h"

namespace CatEngine {
class AssetBrowserPanel : public EditorPanel {
public:
  AssetBrowserPanel();

  Ref<Scene> GetSceneContext() { return m_SceneContext; }
  virtual void SetSceneContext(const Ref<Scene> &context) override;

  virtual void OnImGuiRender(bool &isOpen) override;
  virtual void OnEvent(Event &e) override {};

  void SetCurrentDirectory(const std::filesystem::path &dir) { m_CurrentDirectory = dir; }

  void SetSelectionHandle(const AssetHandle &handle = 0) {
    m_SelectionContext = handle;
  }

private:
  void DrawAsset(const AssetHandle &handle);
  void DrawRegularFile(const bool &isDirectory, const std::filesystem::path &fileName);

  void MoveAssetTarget(const std::filesystem::path &targetPath);
  std::filesystem::path ResolveUniqueFilePath(const std::filesystem::path &targetPath, const std::filesystem::path &fileName);

private:
  Ref<Scene> m_SceneContext;
  std::filesystem::path m_CurrentDirectory;

  AssetHandle m_SelectionContext = 0;

  float m_ThumbnailSize = 75.0f;

  friend class ContentBrowserPanel;
};
} // namespace CatEngine
