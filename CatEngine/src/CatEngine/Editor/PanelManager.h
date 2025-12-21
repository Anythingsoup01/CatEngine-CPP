#pragma once

#include "CatEngine/Core/Hash.h"

#include "EditorPanel.h"

namespace CatEngine
{
    struct PanelData
    {
        const char* ID = "";
        Ref<EditorPanel> Panel = nullptr;
        bool IsOpen = false;
    };

    using PanelDataMap = std::unordered_map<uint32_t, PanelData>;

    enum class PanelCategory
    {
        Edit, View, _COUNT
    };

    class PanelManager
    {
    public:
        PanelManager() = default;
        ~PanelManager();

        PanelData* GetPanelData(uint32_t panelID);
        const PanelData* GetPanelData(uint32_t panelID) const;

        void RemovePanel(const char* strID);

        void OnImGuiRender();

        void OnEvent(Event& e);

        void SetSceneContext(const Ref<Scene>& context);

        void Serialize() const;
        void Deserialize();

        PanelDataMap& GetPanels(PanelCategory category) { return m_Panels[(size_t)category]; }
        const PanelDataMap& GetPanels(PanelCategory category) const { return m_Panels[(size_t)category]; }

    public:

        template<typename TPanel>
        Ref<TPanel> AddPanel(PanelCategory category, const PanelData& panelData)
        {
            static_assert(std::is_base_of<EditorPanel, TPanel>::value, "PanelManager::AddPanel requires TPanel to inherit from EditorPanel");

            auto& panelMap = m_Panels[(size_t)category];

            uint32_t id = Hash::GenerateFNVHash(panelData.ID);
            if (panelMap.find(id) != panelMap.end())
            {
                CE_API_ERROR("A panel with ID '{}' already exists!", panelData.ID);
                return nullptr;
            }

            panelMap[id] = panelData;
            return (Ref<TPanel>&)panelData.Panel;
        }

		template<typename TPanel, typename... TArgs>
		Ref<TPanel> AddPanel(PanelCategory category, const char* strID, bool isOpenByDefault, TArgs&&... args)
		{
			return AddPanel<TPanel>(category, PanelData{ strID, CreateRef<TPanel>(std::forward<TArgs>(args)...), isOpenByDefault });
		}

        template<typename TPanel>
		Ref<TPanel> GetPanel(const char* strID)
		{
			static_assert(std::is_base_of<EditorPanel, TPanel>::value, "PanelManager::AddPanel requires TPanel to inherit from EditorPanel");

			uint32_t id = Hash::GenerateFNVHash(strID);

			for (const auto& panelMap : m_Panels)
			{
				if (panelMap.find(id) == panelMap.end())
					continue;

				return (Ref<TPanel>&)panelMap.at(id).Panel;
			}

			CE_API_ERROR("Couldn't find panel with id '{0}'", strID);
			return nullptr;
		}

    private:
        std::array<PanelDataMap, (size_t)PanelCategory::_COUNT> m_Panels;

    };
}

