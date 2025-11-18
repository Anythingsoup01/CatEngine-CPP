#include "cepch.h"
#include "ScriptGlue.h"

#include "CatEngine/Core/Input.h"
#include "CatEngine/Core/KeyCodes.h"
#include "CatEngine/Core/MouseCodes.h"
#include "CatEngine/Math/Math.h"
#include "CatEngine/Scene/Scene.h"
#include "CatEngine/Scene/Entity.h"
#include "ScriptEngine.h"

#include "CatEngine/Physics/Physics2D.h"

#include "CatEngine/Scene/Components/Components.h"
#include <cstring>

namespace CatEngine
{

    static std::unordered_map <std::string, std::function<bool(Entity)>> s_EntityHasComponentFuncs;

#define CE_ADD_INTERNAL_CALL(Name) capy_add_internal_call(#Name, (void*)&Name)

	static Entity GetEntity(UUID entityID)
	{
		Ref<Scene> scene = ScriptEngine::GetSceneContext();
		CE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		CE_ASSERT(entity);
		return entity;
	}

	static bool Input_IsKeyPressed(KeyCode keyCode)
	{
		return Input::IsKeyPressed(keyCode);
	}

	static bool Input_IsKeyReleased(KeyCode keyCode)
	{
		return Input::IsKeyReleased(keyCode);
	}

#pragma region Object

	static bool Object_HasComponent(UUID entityID, const char* componentType)
	{
		const Entity& entity = GetEntity(entityID);
        CE_API_ASSERT(s_EntityHasComponentFuncs.find(componentType) != s_EntityHasComponentFuncs.end(), "Component not found: {}", componentType);
		return s_EntityHasComponentFuncs.at(componentType)(entity);
	}

    static uint64_t Object_FindObjectByName(const std::string& name)
	{
		Ref<Scene> scene = ScriptEngine::GetSceneContext();
		CE_ASSERT(scene);
		Entity entity = scene->FindEntityByName(name.c_str());
		CE_ASSERT(entity);
		
		if (!entity)
			return 0;

		return entity.GetUUID();
	}


    static const char* Object_GetLayer(UUID entityID)
    {
        const Entity& entity = GetEntity(entityID);

        if (!entity.HasComponent<LayerComponent>()) return "ANON";
        
        return entity.GetComponent<LayerComponent>().Layer.c_str();
        
    }

    static void Object_SetLayer(UUID entityID, const char* value)
    {
        const Entity& entity = GetEntity(entityID);

        if (!entity.HasComponent<LayerComponent>()) return;
        
        entity.GetComponent<LayerComponent>().Layer = value;
    }

    static const char* Object_GetTag(UUID entityID)
    {
        const Entity& entity = GetEntity(entityID);

        if (!entity.HasComponent<TagComponent>()) return "ANON";
        
        return entity.GetComponent<TagComponent>().Tag.c_str();
    }

    static void Object_SetTag(UUID entityID, const char* value)
    {
        const Entity& entity = GetEntity(entityID);

        if (!entity.HasComponent<TagComponent>()) return;
        
        entity.GetComponent<TagComponent>().Tag = value;

    }

#pragma endregion

#pragma region Scripts


#pragma endregion


#pragma region Rigidbody2D

	static void Rigidbody2D_ApplyForce(UUID entityID, glm::vec2* impulse, glm::vec2* point, bool wake)
	{
		const Entity& entity = GetEntity(entityID);
		Physics2D::ApplyForce(entity, *impulse, *point, wake);
	}
	static void Rigidbody2D_ApplyForceToCenter(UUID entityID, glm::vec2* impulse, bool wake)
	{
		const Entity& entity = GetEntity(entityID);
		Physics2D::ApplyForceToCenter(entity, *impulse, wake);
	}
	static void Rigidbody2D_ApplyLinearImpulse(UUID entityID, glm::vec2* impulse, glm::vec2* point, bool wake)
	{
		const Entity& entity = GetEntity(entityID);
		Physics2D::ApplyLinearImpulse(entity, *impulse, *point, wake);
	}
	static void Rigidbody2D_ApplyLinearImpulseToCenter(UUID entityID, glm::vec2* impulse, bool wake)
	{
		const Entity& entity = GetEntity(entityID);
		Physics2D::ApplyLinearImpulseToCenter(entity, *impulse, wake);
	}


#pragma endregion

#pragma region Transform

	static void Transform_GetPosition(UUID entityID, glm::vec3* outPosition)
	{
		const Entity& entity = GetEntity(entityID);
		*outPosition = entity.GetComponent<TransformComponent>().Position;
	}

	static void Transform_SetPosition(UUID entityID, glm::vec3* position)
	{
		const Entity& entity = GetEntity(entityID);

		entity.GetComponent<TransformComponent>().Position = *position;
	}

	static void Transform_GetRotation(UUID entityID, glm::vec3* outRotation)
	{
		const Entity& entity = GetEntity(entityID);
		*outRotation = entity.GetComponent<TransformComponent>().Rotation;
	}

	static void Transform_SetRotation(UUID entityID, glm::vec3* rotation)
	{
		const Entity& entity = GetEntity(entityID);

		entity.GetComponent<TransformComponent>().Rotation = *rotation;
	}

	static void Transform_GetScale(UUID entityID, glm::vec3* outScale)
	{
		const Entity& entity = GetEntity(entityID);
		*outScale = entity.GetComponent<TransformComponent>().Scale;
	}

	static void Transform_SetScale(UUID entityID, glm::vec3* scale)
	{
		const Entity& entity = GetEntity(entityID);

		entity.GetComponent<TransformComponent>().Scale = *scale;
	}

#pragma endregion

#pragma region Sprite

    static void Sprite_GetColor(UUID entityID, glm::vec4* outColor)
	{
		const Entity& entity = GetEntity(entityID);
		*outColor = entity.GetComponent<SpriteRendererComponent>().Color;
	}

	static void Sprite_SetColor(UUID entityID, glm::vec4* color)
	{
		const Entity& entity = GetEntity(entityID);
		entity.GetComponent<SpriteRendererComponent>().Color = *color;
	}

#pragma endregion


	template<typename ... Component>
	static void RegisterComponent()
	{

		([]()
		{
			const char* typeName = ComponentToString<Component>();

            std::string name(typeName);
            name = name.substr(0, name.length() - strlen("Component"));

			s_EntityHasComponentFuncs[name] = [](Entity entity) { return entity.HasComponent<Component>(); };
		}(), ...);
	}

	template<typename ... Component>
	static void RegisterComponent(ComponentGroup<Component ...>)
	{
		RegisterComponent<Component ...>();
	}

	void ScriptGlue::RegisterComponents()
	{
		s_EntityHasComponentFuncs.clear();
		RegisterComponent(AllComponents{});
	}

	void ScriptGlue::RegisterFunctions()
	{
		CE_ADD_INTERNAL_CALL(Input_IsKeyPressed);
		CE_ADD_INTERNAL_CALL(Input_IsKeyReleased);

#pragma region Object

        CE_ADD_INTERNAL_CALL(Object_HasComponent);
        CE_ADD_INTERNAL_CALL(Object_FindObjectByName);
        CE_ADD_INTERNAL_CALL(Object_GetLayer);
        CE_ADD_INTERNAL_CALL(Object_SetLayer);
        CE_ADD_INTERNAL_CALL(Object_GetTag);
        CE_ADD_INTERNAL_CALL(Object_SetTag);

#pragma endregion

#pragma region Scripts


#pragma endregion


#pragma region Rigidbody2D
        CE_ADD_INTERNAL_CALL(Rigidbody2D_ApplyForce);
        CE_ADD_INTERNAL_CALL(Rigidbody2D_ApplyForceToCenter);
        CE_ADD_INTERNAL_CALL(Rigidbody2D_ApplyLinearImpulse);
        CE_ADD_INTERNAL_CALL(Rigidbody2D_ApplyLinearImpulseToCenter);
#pragma endregion

#pragma region Transform
        CE_ADD_INTERNAL_CALL(Transform_GetPosition);
        CE_ADD_INTERNAL_CALL(Transform_SetPosition);
        CE_ADD_INTERNAL_CALL(Transform_GetRotation);
        CE_ADD_INTERNAL_CALL(Transform_SetRotation);
        CE_ADD_INTERNAL_CALL(Transform_GetScale);
        CE_ADD_INTERNAL_CALL(Transform_SetScale);
#pragma endregion

#pragma region Sprite

        CE_ADD_INTERNAL_CALL(Sprite_GetColor);
	    CE_ADD_INTERNAL_CALL(Sprite_SetColor);

#pragma endregion
	}
}
