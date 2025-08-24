#pragma once
#include "CatEngine.h"


namespace CatEngine
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& context);
		~SceneHierarchyPanel() = default;

		void SetContext(const Ref<Scene>& context);

		void OnImGuiRender();

		Entity GetSelectedEntity() const { return m_SelectionContext; }

		void SetSelectedEntity(Entity entity = {}) { m_SelectionContext = entity; }
	private:
		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity selection);

		template<typename T>
		void DisplayAddComponentEntry(const std::string& entryName);

		template<typename T, typename O>
		void DisplayAddComponentEntries(const std::string& entryOneName, const std::string& entryTwoName);

		void DisplayAddScriptComponent(const std::string& name);

	private:
		Ref<Scene> m_Context;
		Entity m_SelectionContext;
	};

}
