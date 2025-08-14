#include "cepch.h"
#include "Scene.h"

#include "Components/Components.h"
#include "Entity.h"

#include "CatEngine/Renderer/Renderer2D.h"

#include <glm/glm.hpp>

namespace CatEngine
{
    Scene::Scene()
    {
    }

    Scene::~Scene()
    {
    }

    template<typename... Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		([&]()
		{
			auto view = src.view<Component>();
			for (auto srcEntity : view)
			{
				UUID uuid = src.get<IDComponent>(srcEntity).ID;
				CE_API_ASSERT(enttMap.find(uuid) != enttMap.end(), "ID Not found!");
				entt::entity dstEnttID = enttMap.at(uuid);

				auto& component = src.get<Component>(srcEntity);
				dst.emplace_or_replace<Component>(dstEnttID, component);
			}
		}(), ...);
	}

	template<typename... Component>
	static void CopyComponent(ComponentGroup<Component ... >, entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		CopyComponent<Component ...>(dst, src, enttMap);
	}

	template<typename... Component>
	static void CopyComponentIfExists(Entity dst, Entity src)
	{
		([&]()
		{
			if (src.HasComponent<Component>())
				dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
		}(), ...);
	}
	template<typename... Component>
	static void CopyComponentIfExists(ComponentGroup<Component ... >,Entity dst, Entity src)
	{
		CopyComponentIfExists<Component ...>(dst, src);
	}

	Ref<Scene> Scene::Copy(Ref<Scene> src)
	{
		CE_PROFILE_FUNCTION();

		Ref<Scene> dst = CreateRef<Scene>();

		dst->m_ViewportWidth = src->m_ViewportWidth;
		dst->m_ViewportHeight = src->m_ViewportHeight;

		auto& srcSceneRegistry = src->m_Registry;
		auto& dstSceneRegistry = dst->m_Registry;
		std::unordered_map<UUID, entt::entity> enttMap;

		// Create Entities in new scene
		auto idView = srcSceneRegistry.view<IDComponent>();
		for (auto e : idView)
		{
			UUID uuid = srcSceneRegistry.get<IDComponent>(e).ID;
			const auto& name = srcSceneRegistry.get<NameComponent>(e).Name;
			enttMap[uuid] = dst->CreateEntityWithUUID(uuid, name);
		}

		// Copy Components (except IDComponent & NameComponent);

		CopyComponent(AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap);

		return dst;

	}

    void Scene::OnUpdateEditor(Time ts, EditorCamera& camera)
    {
        CE_PROFILE_FUNCTION();
        Renderer2D::BeginScene(camera);
        {
            CE_PROFILE_SCOPE("Draw Sprites");
            auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
            for(auto entity : group)
            {
                auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
                Renderer2D::DrawQuad(transform.GetTransform(), sprite.Color, sprite.Texture, sprite.TilingFactor);
            }
        }

        Renderer2D::EndScene();
    }

    void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		if (m_ViewportWidth == width && m_ViewportHeight == height)
			return;

		m_ViewportWidth = width;
		m_ViewportHeight = height;

		// Resize non-fixed aspect ratios
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if (!cameraComponent.FixedAspectRatio)
			{
				cameraComponent.Camera.SetViewportSize(width, height);
			}
		}
	}



	void Scene::DuplicateEntity(Entity entity)
	{
		PasteEntity(entity);
	}

	Entity Scene::PasteEntity(Entity entity)
	{
		CE_PROFILE_FUNCTION();

		Entity newEntity = CreateEntity(entity.GetName() + "(Copied from " + entity.GetName() + ")");
		CopyComponentIfExists(AllComponents{}, newEntity, entity);
		return newEntity;
	}
	Entity Scene::FindEntityByName(std::string_view name)
	{
		auto view = m_Registry.view<NameComponent>();
		for (auto entity : view)
		{
			const NameComponent& nameComponent = view.get<NameComponent>(entity);
			if (nameComponent.Name == name)
			{
				return Entity{ entity, this };
			}
		}
		return {};
	}
	Entity Scene::GetEntityByUUID(UUID entityID)
	{
		if (m_EntityMap.find(entityID) != m_EntityMap.end())
			return { m_EntityMap.at(entityID), this };
		return {};
	}
	Entity Scene::GetPrimaryCameraEntity()
	{
		CE_PROFILE_FUNCTION();

		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			const auto& cameraComponent = view.get<CameraComponent>(entity);
			if (cameraComponent.Primary)
				return Entity{ entity, this };
		}
		return {};
	}
	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}
	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
	{
		CE_PROFILE_FUNCTION();

		Entity entity = { m_Registry.create(), this };

		entity.AddComponent<IDComponent>(uuid);
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = "None";
		auto& layer = entity.AddComponent<LayerComponent>();
		layer.Layer = "Default";
		auto& entityName = entity.AddComponent<NameComponent>();
		entityName.Name = name.empty() ? "GameObject" : name;

		entity.AddComponent<TransformComponent>();

		m_EntityMap[uuid] = entity.GetEntityID();

		return entity;
	}
	void Scene::DeleteEntity(Entity entity)
	{
		m_Registry.destroy(entity);
		m_EntityMap.erase(entity.GetUUID());
	}


	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		static_assert(false);
	}
	template<>
	void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component)
	{

	}
	template<>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{
	}
	template<>
	void Scene::OnComponentAdded<LayerComponent>(Entity entity, LayerComponent& component)
	{
	}
	template<>
	void Scene::OnComponentAdded<NameComponent>(Entity entity, NameComponent& component)
	{
	}
	template<>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{
	}
	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
			component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}
	template<>
	void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
	{
	}
	template<>
	void Scene::OnComponentAdded<ScriptComponent>(Entity entity, ScriptComponent& component)
	{
	}

}
