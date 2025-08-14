#pragma once

#include "CatEngine/Renderer/Camera.h"

namespace CatEngine
{
	class SceneCamera : public Camera
	{
	public:
		enum class ProjectionType { Perspectice = 0, Orthographic = 1};
	public:
		SceneCamera();
		virtual ~SceneCamera() = default;

		void SetOrthographic(float size, float nearClip, float farClip);
		void SetPerspective(float verticalFov, float nearClip, float farClip);
		void SetViewportSize(uint32_t width, uint32_t height);
		void CheckViewportSize();

		float GetOrthographicSize() const { return m_OrthographicSize; }
		void SetOrthographicSize(float size) { m_OrthographicSize = size; RecalculateProjection(); }
		float GetOrthographicNearClip() const { return m_OrthographicNear; }
		void SetOrthographicNearClip(float nearClip) { m_OrthographicNear = nearClip; RecalculateProjection(); }
		float GetOrthographicFarClip() const { return m_OrthographicFar; }
		void SetOrthographicFarClip(float farClip) { m_OrthographicFar = farClip; RecalculateProjection(); }

		float GetPerspectiveVerticalFov() const { return m_PerspectiveFOV; }
		void SetPerspectiveVerticalFov(float verticalFov) { m_PerspectiveFOV = verticalFov; RecalculateProjection(); }
		float GetPerspectiveNearClip() const { return m_PerspectiveNear; }
		void SetPerspectiveNearClip(float nearClip) { m_PerspectiveNear = nearClip; RecalculateProjection(); }
		float GetPerspectiveFarClip() const { return m_PerspectiveFar; }
		void SetPerspectiveFarClip(float farClip) { m_PerspectiveFar = farClip; RecalculateProjection(); }

		float GetAspectRatio() const { return m_AspectRatio; }
		void SetAspectRatio(float aspectRatio) { m_AspectRatio = aspectRatio; }

		ProjectionType GetProjectionType() const { return m_ProjectionType; }
		void SetProjectionType(ProjectionType projectionType) { m_ProjectionType = projectionType; RecalculateProjection(); }
	private:
		friend class Scene;
	private:
		void RecalculateProjection();
	private:
		ProjectionType m_ProjectionType = ProjectionType::Orthographic;

		float m_OrthographicSize = 10.f;
		float m_OrthographicNear = -1.f, m_OrthographicFar = 1.f;

		float m_PerspectiveFOV = glm::radians(45.f);
		float m_PerspectiveNear = 0.01f, m_PerspectiveFar = 10000.f;

		float m_AspectRatio = 0.f;
	};
}
