#include "cepch.h"
#include "ScriptGlue.h"

#include "ScriptEngine.h"

namespace CatEngine
{
    Entity ScriptGlue::GetEntity(UUID entityID)
	{
		Ref<Scene> scene = ScriptEngine::GetSceneContext();
		CE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		CE_ASSERT(entity);
		return entity;
	}

	bool ScriptGlue::Input_IsKeyDown(KeyCode keyCode)
	{
		return Input::IsKeyPressed(keyCode);
	}
#pragma region Object

    template<typename T>
	bool ScriptGlue::Object_HasComponent(UUID entityID, T component)
	{
		Entity entity = GetEntity(entityID);
        return entity.HasComponent<T>();
	}

	uint64_t ScriptGlue::Object_FindObjectByName(const std::string& name)
	{
		Ref<Scene> scene = ScriptEngine::GetSceneContext();
		CE_ASSERT(scene);
		Entity entity = scene->FindEntityByName(name.c_str());
		CE_ASSERT(entity);
		
		if (!entity)
			return 0;

		return entity.GetUUID();

	}

#pragma endregion

#pragma region Scripts

	void* ScriptGlue::GetScriptInstance(UUID entityID)
	{
		return ScriptEngine::GetManagedInstance(entityID);
	}

#pragma endregion


#pragma region Rigidbody2D

	void ScriptGlue::Rigidbody2D_ApplyForce(UUID entityID, glm::vec2* impulse, glm::vec2* point, bool wake)
	{
		Entity entity = GetEntity(entityID);
		//Physics2D::ApplyForce(entity, *impulse, *point, wake);
	}
	void ScriptGlue::Rigidbody2D_ApplyForceToCenter(UUID entityID, glm::vec2* impulse, bool wake)
	{
		Entity entity = GetEntity(entityID);
		//Physics2D::ApplyForceToCenter(entity, *impulse, wake);
	}
	void ScriptGlue::Rigidbody2D_ApplyLinearImpulse(UUID entityID, glm::vec2* impulse, glm::vec2* point, bool wake)
	{
		Entity entity = GetEntity(entityID);
		//Physics2D::ApplyLinearImpulse(entity, *impulse, *point, wake);
	}
	void ScriptGlue::Rigidbody2D_ApplyLinearImpulseToCenter(UUID entityID, glm::vec2* impulse, bool wake)
	{
		Entity entity = GetEntity(entityID);
		//Physics2D::ApplyLinearImpulseToCenter(entity, *impulse, wake);
	}


#pragma endregion

#pragma region Transform

	glm::vec3 ScriptGlue::Transform_GetPosition(UUID entityID)
	{
		Entity entity = GetEntity(entityID);
		return entity.GetComponent<TransformComponent>().Position;
	}

	void ScriptGlue::Transform_SetPosition(UUID entityID, const glm::vec3& position)
	{
		Entity entity = GetEntity(entityID);

		entity.GetComponent<TransformComponent>().Position = position;
	}

	glm::vec3 ScriptGlue::Transform_GetRotation(UUID entityID)
	{
		Entity entity = GetEntity(entityID);
		return entity.GetComponent<TransformComponent>().Rotation;
	}

	void ScriptGlue::Transform_SetRotation(UUID entityID, const glm::vec3& rotation)
	{
		Entity entity = GetEntity(entityID);

		entity.GetComponent<TransformComponent>().Rotation = rotation;
	}

	glm::vec3 ScriptGlue::Transform_GetScale(UUID entityID)
	{
		Entity entity = GetEntity(entityID);
		return entity.GetComponent<TransformComponent>().Scale;
	}

	void ScriptGlue::Transform_SetScale(UUID entityID, const glm::vec3& scale)
	{
		Entity entity = GetEntity(entityID);

		entity.GetComponent<TransformComponent>().Scale = scale;
	}

#pragma endregion

}
