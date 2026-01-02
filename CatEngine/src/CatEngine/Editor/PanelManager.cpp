#include "cepch.h"

#include "CatEngine/Core/Application.h"

#include "PanelManager.h"

#include <yaml-cpp/yaml.h>

namespace CatEngine
{

    PanelManager::~PanelManager()
    {
        for (auto& map : m_Panels)
            map.clear();
    }

    PanelData* PanelManager::GetPanelData(uint32_t panelID)
    {
        for (auto& panelMap : m_Panels)
        {
            if (panelMap.find(panelID) == panelMap.end())
                continue;

            return &panelMap.at(panelID);
        }

        return nullptr;
    }

    const PanelData* PanelManager::GetPanelData(uint32_t panelID) const
    {
        for (auto& panelMap : m_Panels)
        {
            if (panelMap.find(panelID) == panelMap.end())
                continue;

            return &panelMap.at(panelID);
        }

        return nullptr;
    }

    void PanelManager::RemovePanel(const char* strID)
    {
        uint32_t id = Hash::GenerateFNVHash(strID);

        for (auto& panelMap : m_Panels)
        {
            if (panelMap.find(id) == panelMap.end())
                continue;

            panelMap.erase(id);
            return;
        }

        CE_API_ERROR("Couldn't find panels with id {}", strID);
    }

    void PanelManager::OnImGuiRender()
    {
        for (auto panelMap : m_Panels)
        {
            for (auto& [id, panelData] : panelMap)
            {
                bool closedThisFrame = false;

                if (panelData.IsOpen)
                {
                    panelData.Panel->OnImGuiRender(panelData.IsOpen);
                    closedThisFrame = !panelData.IsOpen;
                }

                if (closedThisFrame)
                    Serialize();
            }
        }
    }

    void PanelManager::OnEvent(Event& e)
    {
        for (auto& panelMap : m_Panels)
        {
            for (auto& [id, panelData] : panelMap)
                panelData.Panel->OnEvent(e);
        }
    }

    void PanelManager::SetSceneContext(const Ref<Scene>& context)
    {
        for (auto& panelMap : m_Panels)
        {
            for (auto& [id, panelData] : panelMap)
                panelData.Panel->SetSceneContext(context);
        }
    }

    void PanelManager::Serialize() const
    {
        YAML::Emitter out;
        out << YAML::BeginMap;

        out << YAML::Key << "Panels" << YAML::Value << YAML::BeginSeq;
        {
            for (size_t category = 0; category < m_Panels.size(); category++)
            {
                for (const auto& [panelID, panel] : m_Panels[category])
                {
                    out << YAML::BeginMap;
                    out << YAML::Key << "ID" << YAML::Value << panelID;
                    out << YAML::Key << "IsOpen" << YAML::Value << panel.IsOpen;
                    out << YAML::EndMap;
                }
            }
        }
        out << YAML::EndSeq;

        out << YAML::EndMap;

        std::ofstream fout(Application::Get().GetMainPath() / "EditorLayout.yaml");
        fout << out.c_str();
        fout.close();
    }

    void PanelManager::Deserialize()
    {
        std::filesystem::path layoutPath = Application::Get().GetMainPath() / "EditorLayout.yaml";
        if (!std::filesystem::exists(layoutPath))
            return;

        std::ifstream strean(layoutPath);
        if (!strean.is_open())
            return;

        std::stringstream ss;
        ss << strean.rdbuf();

        YAML::Node data = YAML::Load(ss.str());
        if (!data["Panels"])
        {
            CE_API_ERROR("Failed to load EditorLayout.yaml from {}!", layoutPath.parent_path().string());
            return;
        }

        
		for (auto panelNode : data["Panels"])
		{
			PanelData* panelData = GetPanelData(panelNode["ID"].as<uint32_t>(0));

			if (panelData == nullptr)
				continue;

			panelData->IsOpen = panelNode["IsOpen"].as<bool>(panelData->IsOpen);
		}
    }

}
