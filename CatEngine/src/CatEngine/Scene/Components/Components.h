#pragma once

#include "2D/SpriteRenderer.h"

#include "3D/TransformComponent.h"

#include "Renderer/CameraComponent.h"

#include "Required/InformationComponents.h"

#include "Scripting/ScriptComponent.h"


namespace CatEngine
{
	template<typename... Component>
	struct ComponentGroup
	{

	};

	using AllComponents =
		ComponentGroup<TagComponent, LayerComponent, TransformComponent, CameraComponent,
		SpriteRendererComponent,
		ScriptComponent>;
}
