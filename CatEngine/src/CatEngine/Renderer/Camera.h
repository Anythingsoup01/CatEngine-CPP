#pragma once

#include <glm/glm.hpp>

namespace CatEngine
{
    class OrthographicCamera
    {
    public:
        OrthographicCamera(float left, float right, float bottom, float top, float near = -1.0f, float far = 1.0f);
        
        void SetProjection(float left, float right, float bottom, float top, float near = -1.0f, float far = 1.0f);

        void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateViewMatrix(); }
        void SetRotation(float rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }

        const glm::mat4& GetProjectMatrix() const { return m_ProjectionMatrix; }
        const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
        const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectMatrix; }

    private:
        void RecalculateViewMatrix();
    private:
        glm::mat4 m_ProjectionMatrix;
        glm::mat4 m_ViewMatrix;
        glm::mat4 m_ViewProjectMatrix;

        glm::vec3 m_Position = glm::vec3(1.0f);
        float m_Rotation = 0.0f;
    };
}
