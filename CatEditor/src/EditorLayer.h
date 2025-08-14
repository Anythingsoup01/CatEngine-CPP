#pragma once

#include "CatEngine.h"

#include "imgui.h"

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

        void UI_Viewport();


    private:
        Ref<Texture2D> m_Texture, m_Texture2, m_Texture3;
        EditorCamera m_EditorCamera;

        Entity m_Car;

        Entity m_CarViewer;

        bool m_UsingMainCamera = false;
        float m_DeltaTime;

        Ref<Scene> m_ActiveScene;
        Ref<Framebuffer> m_Framebuffer;
        glm::vec2 m_ViewportSize;
    private:
        static inline EditorLayer* s_Instance = nullptr;
    };

}


