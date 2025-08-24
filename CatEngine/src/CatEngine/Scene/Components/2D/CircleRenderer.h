#pragma once 

#include <glm/glm.hpp>

namespace CatEngine
{
	struct CircleRendererComponent
	{
		glm::vec4 Color{ 1.f, 1.f, 1.f, 1.f };
		float Thickness = 1.0f;
		float Fade = 0.005f;

		CircleRendererComponent() = default;
		CircleRendererComponent(const CircleRendererComponent&) = default;
	};
}
