#pragma once

#include "CatEngine/AssetManager/Asset.h"
#include "CatEngine/Editor/EditorPanel.h"

#include "CatEngine/Scene/Entity.h"
#include "CatEngine/Scene/Scene.h"

namespace CatEngine {

class InspectorPanel : public EditorPanel {
public:
  InspectorPanel() = default;
  InspectorPanel(const Ref<Scene> &context);

  virtual void SetSceneContext(const Ref<Scene> &context) override;
  Ref<Scene> GetSceneContext() { return m_Context; }

  virtual void OnImGuiRender(bool &isOpen) override;
  virtual void OnEvent(Event &e) override {}

  void SetEntitySelectionContext(Entity e);
  void SetAssetSelectionContext(AssetHandle handle);

private:
  void DrawComponents(Entity selection);
  void DrawAsset(const AssetHandle &handle);

  template <typename T>
  void DisplayAddComponentEntry(const std::string &entryName);

  template <typename T, typename O>
  void DisplayAddComponentEntries(const std::string &entryOneName,
                                  const std::string &entryTwoName);

  void DisplayAddScriptComponent(const std::string &name);

private:
  Ref<Scene> m_Context;
  Entity m_EntitySelection = {};
  AssetHandle m_AssetSelection = 0;
};
}
