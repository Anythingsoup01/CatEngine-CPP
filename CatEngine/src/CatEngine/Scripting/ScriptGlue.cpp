#include "cepch.h"
#include "ScriptGlue.h"

#include "ScriptEngine.h"

#include "CatEngine/Physics/Physics2D.h"

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
    template<>
    bool ScriptGlue::Object_HasComponent(UUID entityID, TagComponent component)
    {
		Entity entity = GetEntity(entityID);
        return entity.HasComponent<TagComponent>();
    }
    template<>
    bool ScriptGlue::Object_HasComponent(UUID entityID, LayerComponent component)
    {
		Entity entity = GetEntity(entityID);
        return entity.HasComponent<LayerComponent>();
    }
    template<>
    bool ScriptGlue::Object_HasComponent(UUID entityID, TransformComponent component)
    {
		Entity entity = GetEntity(entityID);
        return entity.HasComponent<TransformComponent>();
    }
    template<>
    bool ScriptGlue::Object_HasComponent(UUID entityID, CameraComponent component)
    {
		Entity entity = GetEntity(entityID);
        return entity.HasComponent<CameraComponent>();
    }
    template<>
    bool ScriptGlue::Object_HasComponent(UUID entityID, SpriteRendererComponent component)
    {
		Entity entity = GetEntity(entityID);
        return entity.HasComponent<SpriteRendererComponent>();
    }
    template<>
    bool ScriptGlue::Object_HasComponent(UUID entityID, CircleRendererComponent component)
    {
		Entity entity = GetEntity(entityID);
        return entity.HasComponent<CircleRendererComponent>();
    }
    template<>
    bool ScriptGlue::Object_HasComponent(UUID entityID, BoxCollider2DComponent component)
    {
		Entity entity = GetEntity(entityID);
        return entity.HasComponent<BoxCollider2DComponent>();
    }
    template<>
    bool ScriptGlue::Object_HasComponent(UUID entityID, CircleCollider2DComponent component)
    {
		Entity entity = GetEntity(entityID);
        return entity.HasComponent<CircleCollider2DComponent>();
    }
    template<>
    bool ScriptGlue::Object_HasComponent(UUID entityID, Rigidbody2DComponent component)
    {
		Entity entity = GetEntity(entityID);
        return entity.HasComponent<Rigidbody2DComponent>();
    }
    template<>
    bool ScriptGlue::Object_HasComponent(UUID entityID, ScriptComponent component)
    {
		Entity entity = GetEntity(entityID);
        return entity.HasComponent<ScriptComponent>();
    }

    template<typename T>
	T ScriptGlue::Object_GetComponent(UUID entityID)
	{
        static_assert(false, "Component not supported!");
	}
    template<>
    TagComponent ScriptGlue::Object_GetComponent<TagComponent>(UUID entityID)
    {
		Entity entity = GetEntity(entityID);
        if (entity.HasComponent<TagComponent>())
            return entity.GetComponent<TagComponent>();
        CE_API_ERROR("Entity does not have component!");
        return TagComponent();
    }
    template<>
    LayerComponent ScriptGlue::Object_GetComponent<LayerComponent>(UUID entityID)
    {
		Entity entity = GetEntity(entityID);
        if (entity.HasComponent<LayerComponent>())
            return entity.GetComponent<LayerComponent>();
        CE_API_ERROR("Entity does not have component!");
        return LayerComponent();
    }
    template<>
    TransformComponent ScriptGlue::Object_GetComponent<TransformComponent>(UUID entityID)
    {
		Entity entity = GetEntity(entityID);
        if (entity.HasComponent<TransformComponent>())
            return entity.GetComponent<TransformComponent>();
        CE_API_ERROR("Entity does not have component!");
        return TransformComponent();
    }
    template<>
    CameraComponent ScriptGlue::Object_GetComponent<CameraComponent>(UUID entityID)
    {
		Entity entity = GetEntity(entityID);
        if (entity.HasComponent<CameraComponent>())
            return entity.GetComponent<CameraComponent>();
        CE_API_ERROR("Entity does not have component!");
        return CameraComponent();
    }
    template<>
    SpriteRendererComponent ScriptGlue::Object_GetComponent<SpriteRendererComponent>(UUID entityID)
    {
		Entity entity = GetEntity(entityID);
        if (entity.HasComponent<SpriteRendererComponent>())
            return entity.GetComponent<SpriteRendererComponent>();
        CE_API_ERROR("Entity does not have component!");
        return SpriteRendererComponent();
    }
    template<>
    CircleRendererComponent ScriptGlue::Object_GetComponent<CircleRendererComponent>(UUID entityID)
    {
		Entity entity = GetEntity(entityID);
        if (entity.HasComponent<CircleRendererComponent>())
            return entity.GetComponent<CircleRendererComponent>();
        CE_API_ERROR("Entity does not have component!");
        return CircleRendererComponent();
    }
    template<>
    BoxCollider2DComponent ScriptGlue::Object_GetComponent<BoxCollider2DComponent>(UUID entityID)
    {
		Entity entity = GetEntity(entityID);
        if (entity.HasComponent<BoxCollider2DComponent>())
            return entity.GetComponent<BoxCollider2DComponent>();
        CE_API_ERROR("Entity does not have component!");
        return BoxCollider2DComponent();
    }
    template<>
    CircleCollider2DComponent ScriptGlue::Object_GetComponent<CircleCollider2DComponent>(UUID entityID)
    {
		Entity entity = GetEntity(entityID);
        if (entity.HasComponent<CircleCollider2DComponent>())
            return entity.GetComponent<CircleCollider2DComponent>();
        CE_API_ERROR("Entity does not have component!");
        return CircleCollider2DComponent();
    }
    template<>
    Rigidbody2DComponent ScriptGlue::Object_GetComponent<Rigidbody2DComponent>(UUID entityID)
    {
		Entity entity = GetEntity(entityID);
        if (entity.HasComponent<Rigidbody2DComponent>())
            return entity.GetComponent<Rigidbody2DComponent>();
        CE_API_ERROR("Entity does not have component!");
        return Rigidbody2DComponent();
    }
    template<>
    ScriptComponent ScriptGlue::Object_GetComponent<ScriptComponent>(UUID entityID)
    {
		Entity entity = GetEntity(entityID);
        if (entity.HasComponent<ScriptComponent>())
            return entity.GetComponent<ScriptComponent>();
        CE_API_ERROR("Entity does not have component!");
        return ScriptComponent();
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
		//return ScriptEngine::GetManagedInstance(entityID);
        return nullptr;
	}

#pragma endregion


#pragma region Rigidbody2D

	void ScriptGlue::Rigidbody2D_ApplyForce(Rigidbody2DComponent& rb2d, const glm::vec2* impulse, const glm::vec2* point, bool wake)
	{
		Physics2D::ApplyForce(rb2d, *impulse, *point, wake);
	}
	void ScriptGlue::Rigidbody2D_ApplyForceToCenter(Rigidbody2DComponent& rb2d, const glm::vec2* impulse, bool wake)
	{
		Physics2D::ApplyForceToCenter(rb2d, *impulse, wake);
	}
	void ScriptGlue::Rigidbody2D_ApplyLinearImpulse(Rigidbody2DComponent& rb2d, const glm::vec2* impulse, const glm::vec2* point, bool wake)
	{
		Physics2D::ApplyLinearImpulse(rb2d, *impulse, *point, wake);
	}
	void ScriptGlue::Rigidbody2D_ApplyLinearImpulseToCenter(Rigidbody2DComponent& rb2d, const glm::vec2* impulse, bool wake)
	{
		Physics2D::ApplyLinearImpulseToCenter(rb2d, *impulse, wake);
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
