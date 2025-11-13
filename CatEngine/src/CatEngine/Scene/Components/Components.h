#pragma once

#include "2D/SpriteRenderer.h"
#include "2D/CircleRenderer.h"

#include "3D/TransformComponent.h"

#include "Renderer/CameraComponent.h"

#include "Physics/BoxCollider2D.h"
#include "Physics/CircleCollider2D.h"
#include "Physics/Rigidbody2D.h"

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
		SpriteRendererComponent, CircleRendererComponent,
        BoxCollider2DComponent, CircleCollider2DComponent, Rigidbody2DComponent,
		ScriptComponent>;

    template<typename T>
    const char* ComponentToString() { return "NULL"; }

    template<> const char* ComponentToString<TagComponent>();


}
