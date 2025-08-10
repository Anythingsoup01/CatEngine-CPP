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

        static void BeginScene(OrthographicCamera& camera);
        static void EndScene();

        static void Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform = glm::mat4(1.0f));

    private:
        struct SceneData
        {
            glm::mat4 ViewProjectionMatrix;
        };
        inline static SceneData* s_SceneData = new SceneData;
    };
}
