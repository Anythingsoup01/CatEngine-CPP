#pragma once

#include "CatEngine.h"

#include "imgui.h"

#include "Panels/ContentBrowserPanel.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/SceneViewportPanel.h"

namespace CatEngine
{
    class EditorLayer : public Layer
    {
    public:
        virtual void OnAttach() override;
        virtual void OnDetach() override;

        virtual void OnUpdate(Time deltaTime) override;
        virtual void OnImGuiDraw() override;
        virtual void OnEvent(Event& event) override;

        static EditorLayer& Get() { return *s_Instance; }
    private:
        bool OnWindowResize(WindowResizeEvent& e);
        bool OnKeyPressed(KeyPressedEvent& e);
        bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

        void ImGuizmoDraw(Entity selectedEntity, const glm::mat4& cameraProjection, glm::mat4 cameraView);

        void UI_Viewport();
        void UI_Gizmos();


        void OpenScene(const std::filesystem::path& filePath);
        void SaveScene(const std::filesystem::path& filePath); // TODO: STORE SCENE PATH

    private:

        std::filesystem::path m_SceneFilePath;

        EditorCamera m_EditorCamera;

        bool m_UsingMainCamera = false;
        float m_DeltaTime;

        Ref<Scene> m_ActiveScene;
        Ref<Scene> m_EditorScene;

        Entity m_HoveredEntity;

        SceneViewportPanel m_SceneViewportPanel;
        SceneViewportPanel m_SceneCameraPanel;
        SceneHierarchyPanel m_SceneHierarchyPanel;
        ContentBrowserPanel m_ContentBrowserPanel;
		
        int m_GizmoType = -1;

    private:
        static inline EditorLayer* s_Instance = nullptr;
    };

}


