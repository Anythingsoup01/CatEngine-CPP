#pragma once

#include "CatEngine.h"

#include "imgui.h"

#include "Panels/SceneViewportPanel.h"

#include "CatEngine/Editor/PanelManager.h"

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

        void OnUpdateEditor(Time deltaTime);
        void OnUpdateMainCameraPreview(Time deltaTime);
        void OnUpdateRuntime(Time deltaTime);
        void OnUpdateSimulation(Time deltaTime);

        void OnOverlayDraw();

        void ImGuizmoDraw(Entity selectedEntity, const glm::mat4& cameraProjection, glm::mat4 cameraView);

        void UI_Viewport();
        void UI_Gizmos();
        void UI_Toolbar();
        void UI_MenuBar();

		void SaveSceneAs();
		void SaveScene();

		void OpenScene(AssetHandle handle);
		
		void NewScene();

		void SaveProject();

		void OpenProject();
		void OpenProject(const std::filesystem::path& filePath);
		
		void NewProject();
		
		void OnScenePlay();
		void OnScenePause(bool isPaused);
		void OnSceneStop();

		void OnSceneSimulateStart();
		void OnSceneSimulateStop();
		
        void DuplicateEntity();
		void DeleteEntity();
		void CopyEntity();
		void PasteEntity();

		void OnOverlayRender();

    private:
        Scope<PanelManager> m_PanelManager;

        std::filesystem::path m_CurrentProjectPath;
        std::filesystem::path m_EditorScenePath;

        EditorCamera m_EditorCamera;

        bool m_BlockKeyboardEvents = false;
		bool m_BlockMouseEvents = false;
        
        float m_DeltaTime;

        bool m_MouseInUse = false;
		bool m_MouseInUseFlag = false;

		bool m_IsScenePaused = false;

        bool m_ProjectActive = false;

        Ref<Scene> m_CurrentScene;

        Ref<Scene> m_RuntimeScene;
        Ref<Scene> m_SimulationScene;
        Ref<Scene> m_EditorScene;

        AssetHandle m_CurrentSceneHandle = 0;

        Entity m_HoveredEntity;
        Entity m_CopiedEntity;

        SceneViewportPanel m_SceneViewportPanel;
        SceneViewportPanel m_SceneCameraPanel;
		
        int m_GizmoType = -1;

        enum class SceneState
		{
			Edit = 0,
			Play = 1,
			Simulate = 2,
			Pause = 3
		};
		SceneState m_SceneState = SceneState::Edit;

        Ref<Texture2D> m_IconStartRuntime, m_IconPauseRuntime, m_IconPauseRuntimeSelected, m_IconNextFrameRuntime, m_IconStopRuntime;
		Ref<Texture2D> m_IconStartSimulation;

    private:
        static inline EditorLayer* s_Instance = nullptr;
    };

}


