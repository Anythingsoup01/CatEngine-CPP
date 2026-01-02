#pragma once

#include "CatEngine/Editor/EditorPanel.h"

#include "CatEngine/Scene/Scene.h"
#include "CatEngine/Scene/Entity.h"

namespace CatEngine
{
    class SceneHierarchyPanel : public EditorPanel
    {
    public:
        SceneHierarchyPanel() = default;
        SceneHierarchyPanel(const Ref<Scene>& context);

        virtual void SetSceneContext(const Ref<Scene>& context) override;
        Ref<Scene> GetSceneContext() { return m_Context; }

        virtual void OnImGuiRender(bool& isOpen) override;
        virtual void OnEvent(Event& e ) override {}

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
		Entity m_PreviousSelection;
    };

}
