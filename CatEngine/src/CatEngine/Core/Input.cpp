#include "cepch.h"
#include "Input.h"
#include "Application.h"

#include "GLFW/glfw3.h"

namespace CatEngine {
	bool Input::IsKeyPressed(KeyCode keycode)
	{
        CE_PROFILE_FUNCTION();
		auto window = Application::Get().GetWindow().Get();
		auto state = glfwGetKey(window, static_cast<int32_t>(keycode));
		return state == GLFW_PRESS;
	}

	bool Input::IsKeyReleased(KeyCode keycode)
	{
        CE_PROFILE_FUNCTION();
		auto window = Application::Get().GetWindow().Get();
		auto state = glfwGetKey(window, static_cast<int32_t>(keycode));
		return state == GLFW_RELEASE;
	}

	bool Input::IsMouseButtonPressed(MouseCode button)
	{
        CE_PROFILE_FUNCTION();
		auto window = Application::Get().GetWindow().Get();
		auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));
		return state == GLFW_PRESS;
	}

	bool Input::IsMouseButtonReleased(MouseCode button)
	{
        CE_PROFILE_FUNCTION();
		auto window = Application::Get().GetWindow().Get();
		auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));
		return state == GLFW_RELEASE;
	}

	std::pair<float, float> Input::GetMousePosition()
	{
        CE_PROFILE_FUNCTION();
		auto window = Application::Get().GetWindow().Get();
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return { (float)xpos, (float)ypos };
	}

	float Input::GetMouseX()
	{
        CE_PROFILE_FUNCTION();
		auto[x, y] = GetMousePosition();

		return x;
	}

	float Input::GetMouseY()
	{
        CE_PROFILE_FUNCTION();
		auto [x, y] = GetMousePosition();
		return y;
	}
	float Input::GetMouseXOffset()
	{
        CE_PROFILE_FUNCTION();
		float offset = 0;

		if (m_MouseOffsetX == 0)
		{
			m_MouseOffsetX = GetMouseX();
		}
		else if (m_MouseOffsetX != GetMouseX())
		{
			offset = m_MouseOffsetX - GetMouseX();
			m_MouseOffsetX = 0;
		}

		return offset;
	}
	float Input::GetMouseYOffset()
	{
        CE_PROFILE_FUNCTION();
		float offset = 0;

		if (m_MouseOffsetY == 0)
		{
			m_MouseOffsetY = GetMouseY();
		}
		else if (m_MouseOffsetY != GetMouseY())
		{
			offset = m_MouseOffsetY - GetMouseY();
			m_MouseOffsetY = 0;
		}
		return offset;
	}
}
