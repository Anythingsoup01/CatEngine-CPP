#pragma once 

#include "Camera.h"
#include "EditorCamera.h"

#include "Texture.h"

#include <glm/glm.hpp>

namespace CatEngine
{
    class Renderer2D
    {
    public:
        static void Init();
        static void Shutdown();

		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void BeginScene(const EditorCamera& camera);
        static void BeginScene(const OrthographicCamera& camera);
        static void EndScene();

        static void StartBatch();
        static void Flush();

        static void NextBatch();

        // Primitives
        static void DrawQuad(const glm::vec3& position, float rotation, const glm::vec2& size, const glm::vec4& color = glm::vec4(1.0f), const Ref<Texture2D> texture = nullptr, float tilingFactor = 1.0f);
        static void DrawQuad(const glm::mat4& transform, const glm::vec4& color = glm::vec4(1.0f), Ref<Texture2D> texture = nullptr, float tilingFactor = 1.0f);

        struct Statistics
        {
            uint32_t DrawCalls = 0;
            uint32_t QuadCount = 0;

            uint32_t GetTotalVertexCount() const { return QuadCount * 4; }
            uint32_t GetTotalIndexCount() const { return QuadCount * 6; }
        };

        static void ResetStats();
        static Statistics GetStats();

    private:
        static inline bool m_SceneActive = false;

    };
};
