#pragma once

#include "CatEngine/Core/Hash.h"

#include "EditorPanel.h"
#include <cstdint>

namespace CatEngine {
struct PanelData {
  std::string ID = "";
  Ref<EditorPanel> Panel = nullptr;
  bool IsOpen = false;
};

class PanelManager {
public:
  PanelManager();
  ~PanelManager();

  PanelData &GetPanelData(const std::string &panelID);
  const PanelData &GetPanelData(const std::string &panelID) const;

  void RemovePanel(const std::string &panelID);

  void OnImGuiRender();

  void OnEvent(Event &e);

  void SetSceneContext(const Ref<Scene> &context);

  void Serialize() const;
  void Deserialize();

  static PanelManager &Get() { return *s_Instance; }

  template <typename TPanel>
  Ref<TPanel> AddPanel(const PanelData &panelData) {
    static_assert(
        std::is_base_of<EditorPanel, TPanel>::value,
        "PanelManager::AddPanel requires TPanel to inherit from EditorPanel");


    // Instead of returning null or breaking, we just return the already existing panel
    // that way we can have multiple instances of the same panel.
    // Though as of right now they will be exactly the same,
    // I plan to make it to where they can be locked, allowing the user
    // to see specific entities only
    for (auto &panel : m_Panels) {
      if (panel.ID == panelData.ID) {
        return (Ref<TPanel> &)panel.Panel;
      }
    }

    m_Panels.push_back(panelData);
    return (Ref<TPanel> &)panelData.Panel;
  }

  template <typename TPanel, typename... TArgs>
  Ref<TPanel> AddPanel(const char *strID,
                       bool isOpenByDefault, TArgs &&...args) {
    return AddPanel<TPanel>(
        PanelData{strID, CreateRef<TPanel>(std::forward<TArgs>(args)...),
                  isOpenByDefault});
  }

  template <typename TPanel> Ref<TPanel> GetPanel(const std::string &strID) {
    static_assert(
        std::is_base_of<EditorPanel, TPanel>::value,
        "PanelManager::AddPanel requires TPanel to inherit from EditorPanel");

    uint32_t id = Hash::GenerateFNVHash(strID);

    for (auto &panel : m_Panels) {
      if (panel.ID == strID) {
        return (Ref<TPanel> &)panel.Panel;
      }
    }

    CE_API_ERROR("Couldn't find panel with id '{0}'", strID.c_str());
    return nullptr;
  }

private:
  std::vector<PanelData> m_Panels;
  static inline PanelManager *s_Instance;
};
} // namespace CatEngine
