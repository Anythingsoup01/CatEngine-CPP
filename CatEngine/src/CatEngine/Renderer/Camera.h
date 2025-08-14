#pragma once


#include "CatEngine/Core/TimeStep.h"
#include "CatEngine/Event/Event.h"
#include "CatEngine/Event/ApplicationEvent.h"
#include "CatEngine/Event/MouseEvent.h"

#include <glm/glm.hpp>

namespace CatEngine
{	
    class Camera
	{
	public:
		Camera() = default;
		Camera(const glm::mat4& projection)
			: m_Projection(projection) {}

		virtual ~Camera() = default;

		const glm::mat4& GetProjection() const { return m_Projection; }
	protected:
		glm::mat4 m_Projection = glm::mat4(1.f);
	};
    
    class OrthographicCamera
    {
    public:
		OrthographicCamera(float aspectRatio, float Near = -1.f, float Far = 1.f);

        void OnUpdate(Time ts);

        void OnEvent(Event& e);

		void SetProjection(float left, float right, float bottom, float top, float Near = -1.f, float Far = 1.f);

		const glm::vec3 GetPosition() const { return m_Position; }
		const float GetRotation() const  { return m_Rotation; }

		void SetPosition(const glm::vec3& position) 
		{
			m_Position = position;
			RecalculateViewMatrix();
		}
		void SetRotation(float rotation) 
		{
			m_Rotation = rotation;
			RecalculateViewMatrix();
		}
		void OnResize(uint32_t width, uint32_t height) 
		{
			m_AspectRatio = (float)width / (float)height;
			SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		}

		const glm::mat4& GetProjectionMatrix() const        { return m_ProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const              { return m_ViewMatrix; }
		const glm::mat4& GetViewProjectionMatrix() const    { return m_ViewProjectionMatrix; }


	private:
		void RecalculateViewMatrix();

        bool OnWindowResize(WindowResizeEvent& e);
        bool OnMouseScroll(MouseScrolledEvent& e);
	private:
        float m_AspectRatio;
        float m_ZoomLevel = 1.0f;
        float m_CameraSpeed = 1.0f;

        glm::vec2 m_CameraVector;

        int m_Steps = 10;
        


		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ViewProjectionMatrix;

		glm::vec3 m_Position = { 0.f,0.f,0.f };
		float m_Rotation = 0.f;
    };
}
