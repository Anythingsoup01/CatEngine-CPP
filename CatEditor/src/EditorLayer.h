#pragma once

#include "CatEngine.h"

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
    private:

        Ref<Texture> m_Texture, m_TestTexture2;
        Ref<Shader> m_TestShader;
        Ref<VertexArray> m_TestVAO, m_TestVAO2;

        float m_DeltaTime = 0;

        int m_GridSize[2] = { 1, 1 };
            
        glm::vec3 m_CameraPos;
    private:
        static inline EditorLayer* s_Instance = nullptr;
    };
}
