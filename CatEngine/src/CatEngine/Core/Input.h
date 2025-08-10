#pragma once

#include "KeyCodes.h"
#include "MouseCodes.h"

namespace CatEngine {

	class Input {
	protected:
		Input() = default;
	public:
		Input(const Input&) = delete;
		Input& operator=(const Input&) = delete;

		static bool IsKeyPressed(KeyCode keycode);
		static bool IsKeyReleased(KeyCode keycode);

		static bool IsMouseButtonPressed(MouseCode button);
		static bool IsMouseButtonReleased(MouseCode button);
		static std::pair<float, float> GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();

		static float GetMouseXOffset();
		static float GetMouseYOffset();

		static void ResetMouseOffset() { m_MouseOffsetX = 0; m_MouseOffsetY = 0; }

	private:
		static inline float m_MouseOffsetX = 0, m_MouseOffsetY = 0;
	};
}
