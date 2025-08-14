#include "cepch.h"
#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

#include "CatEngine/Core/Input.h"

namespace CatEngine
{
    OrthographicCamera::OrthographicCamera(float aspectRatio, float Near, float Far)
        : m_AspectRatio(aspectRatio), m_ProjectionMatrix(glm::ortho(-aspectRatio * m_ZoomLevel, aspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel, Near, Far)), m_ViewMatrix(1.0f)
    {
        CE_PROFILE_FUNCTION();
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

    void OrthographicCamera::OnUpdate(Time ts)
    {
        CE_PROFILE_FUNCTION();
        if (Input::IsKeyPressed(KeyCode::W))
            m_Position.y += m_CameraSpeed * ts;

        if (Input::IsKeyPressed(KeyCode::S))
            m_Position.y -= m_CameraSpeed * ts;

        if (Input::IsKeyPressed(KeyCode::A))
            m_Position.x -= m_CameraSpeed * ts;

        if (Input::IsKeyPressed(KeyCode::D))
            m_Position.x += m_CameraSpeed * ts;

        RecalculateViewMatrix();
    }

    void OrthographicCamera::OnEvent(Event& e)
    {
        CE_PROFILE_FUNCTION();
        EventDispatcher dispatcher(e);

        dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(OrthographicCamera::OnMouseScroll));
        dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OrthographicCamera::OnWindowResize));
    }

    void OrthographicCamera::SetProjection(float left, float right, float bottom, float top, float near, float far)
    {
        CE_PROFILE_FUNCTION();
        m_ProjectionMatrix = glm::ortho(left, right, bottom, top, near, far);
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

    void OrthographicCamera::RecalculateViewMatrix()
    {
        CE_PROFILE_FUNCTION();
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position) * glm::rotate(glm::mat4(1.0f), m_Rotation, glm::vec3(1.0f));

        m_ViewMatrix = glm::inverse(transform);
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

    bool OrthographicCamera::OnWindowResize(WindowResizeEvent& e)
    {
        CE_PROFILE_FUNCTION();

        //OnResize(e.GetWidth(), e.GetHeight()); // This isn't needed when we use the framebuffer!

        return false;
    }

    bool OrthographicCamera::OnMouseScroll(MouseScrolledEvent& e)
    {
        m_ZoomLevel -= e.GetYOffset() / (float)m_Steps;
        
        m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);

        m_CameraSpeed = m_ZoomLevel;

        SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
        return false;
    }
}
