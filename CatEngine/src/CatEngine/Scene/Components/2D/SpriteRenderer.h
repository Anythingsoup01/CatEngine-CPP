#pragma once

#include "CatEngine/Core/Core.h"
#include "CatEngine/Renderer/Texture.h"

#include <glm/glm.hpp>

namespace CatEngine
{
	struct SpriteRendererComponent
	{

		glm::vec4 Color{ 1.f, 1.f, 1.f, 1.f };
		Ref<Texture2D> Texture;
		float TilingFactor = 1.0f;

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const glm::vec4& color, const Ref<Texture2D>& texture)
			: Color(color), Texture(texture) {}
	};
}
