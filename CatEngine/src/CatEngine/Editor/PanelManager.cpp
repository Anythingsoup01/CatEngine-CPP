#include "cepch.h"

#include "CatEngine/Core/Application.h"

#include "PanelManager.h"

#include <yaml-cpp/yaml.h>

namespace CatEngine {

PanelManager::PanelManager() {
  s_Instance = this;
}

PanelManager::~PanelManager() { m_Panels.clear(); }

PanelData &PanelManager::GetPanelData(const std::string &panelID) {
  for (auto &panel : m_Panels) {
    if (panelID == panel.ID)
      return panel;
  }
  static PanelData s_NullPanelData{"NULL_PANEL", nullptr, false};

  return s_NullPanelData;
}

const PanelData &PanelManager::GetPanelData(const std::string &panelID) const {
  for (auto &panel : m_Panels) {
    if (panelID == panel.ID)
      return panel;
  }
  static PanelData s_NullPanelData{"NULL_PANEL", nullptr, false};

  return s_NullPanelData;
}

void PanelManager::RemovePanel(const std::string &strID) {
  for (auto it = m_Panels.begin(); it != m_Panels.end(); it++) {
    auto &panel = *it;
    if (panel.ID != strID)
      continue;

    m_Panels.erase(it);
    return;
  }

  CE_API_ERROR("Couldn't find panels with id {}", strID);
}

void PanelManager::OnImGuiRender() {
  for (auto panel : m_Panels) {
    bool closedThisFrame = false;

    if (panel.IsOpen) {
      panel.Panel->OnImGuiRender(panel.IsOpen);
      closedThisFrame = !panel.IsOpen;
    }

    if (closedThisFrame)
      Serialize();
  }
}

void PanelManager::OnEvent(Event &e) {
  for (auto &panel : m_Panels) {
    panel.Panel->OnEvent(e);
  }
}

void PanelManager::SetSceneContext(const Ref<Scene> &context) {
  for (auto &panel : m_Panels) {
    panel.Panel->SetSceneContext(context);
  }
}

void PanelManager::Serialize() const {
  YAML::Emitter out;
  out << YAML::BeginMap;

  out << YAML::Key << "Panels" << YAML::Value << YAML::BeginSeq;
  for (const auto &panel : m_Panels) {
    out << YAML::BeginMap;
    out << YAML::Key << "ID" << YAML::Value << panel.ID;
    out << YAML::Key << "IsOpen" << YAML::Value << panel.IsOpen;
    out << YAML::EndMap;
  }
  out << YAML::EndSeq;

  out << YAML::EndMap;

  std::ofstream fout(Application::Get().GetMainPath() / "EditorLayout.yaml");
  fout << out.c_str();
  fout.close();
}

void PanelManager::Deserialize() {
  std::filesystem::path layoutPath =
      Application::Get().GetMainPath() / "EditorLayout.yaml";
  if (!std::filesystem::exists(layoutPath))
    return;

  std::ifstream strean(layoutPath);
  if (!strean.is_open())
    return;

  std::stringstream ss;
  ss << strean.rdbuf();

  YAML::Node data = YAML::Load(ss.str());
  if (!data["Panels"]) {
    CE_API_ERROR("Failed to load EditorLayout.yaml from {}!",
                 layoutPath.parent_path().string());
    return;
  }

  for (auto panelNode : data["Panels"]) {
    PanelData panelData = GetPanelData(panelNode["ID"].as<std::string>());

    if (panelData.ID == "NULL_PANEL")
      continue;

    panelData.IsOpen = panelNode["IsOpen"].as<bool>();
  }
}

} // namespace CatEngine
