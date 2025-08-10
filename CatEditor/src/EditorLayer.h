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
        Ref<Shader> m_Shader;


        Ref<VertexBuffer> m_RedVBO;
        Ref<IndexBuffer> m_RedEBO;
        Ref<VertexArray> m_RedVAO;

        float m_DeltaTime = 0;

        OrthographicCamera m_Camera;

        int m_GridSize[2] = { 1, 1 };
            
        glm::vec3 m_CameraPos;
    private:
        static inline EditorLayer* s_Instance = nullptr;
    };
}
