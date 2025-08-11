#pragma once

#include "VertexArray.h"
#include "Shader.h"
#include "Camera.h"

namespace CatEngine
{
    class Renderer
    {
    public:
        static void Init();

        static void BeginScene();
        static void EndScene();

        static void Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform = glm::mat4(1.0f));

        static void SetCameraPosition(const glm::vec3& position) { m_Camera.SetPosition(position); }
        static glm::vec3 GetCameraPosition() { return m_Camera.GetPosition(); }

    private:
        static inline OrthographicCamera m_Camera = OrthographicCamera(-1.6f, 1.6f, -0.9f, 0.9f);
    };
}
