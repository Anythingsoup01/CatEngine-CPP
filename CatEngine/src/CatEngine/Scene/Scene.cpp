#include "cepch.h"
#include "Scene.h"

#include "Components/Components.h"
#include "Entity.h"

#include "CatEngine/Renderer/Renderer2D.h"

#include <glm/glm.hpp>

#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_circle_shape.h>

#include "CatEngine/Scripting/ScriptEngine.h"

namespace CatEngine
{
    static b2BodyType Rigidbody2DTypeToBox2DType(Rigidbody2DComponent::BodyType bodyType)
	{
		switch (bodyType)
		{
			case CatEngine::Rigidbody2DComponent::BodyType::Static: return b2_staticBody;
			case CatEngine::Rigidbody2DComponent::BodyType::Kinematic: return b2_kinematicBody;
			case CatEngine::Rigidbody2DComponent::BodyType::Dynamic: return b2_dynamicBody;
		}

		CE_API_ASSERT(false, "Unknown body type!");
		return b2_staticBody;
	}

    Scene::Scene()
    {
    }

    Scene::~Scene()
    {
        m_IsRunning = false;
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
		// Draw Sprites
		{
			auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto entity : group)
			{
				auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

				Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
			}
		}
		// Draw Circles
		{
			auto view = m_Registry.view<TransformComponent, CircleRendererComponent>();
			for (auto entity : view)
			{
				auto [transform, circle] = view.get<TransformComponent, CircleRendererComponent>(entity);

				Renderer2D::DrawCircle(transform.GetTransform(), circle, (int)entity);
			}
		}
        Renderer2D::EndScene();
    }
    
    void Scene::OnUpdateMainCameraPreview(Time ts)
    {
        CE_PROFILE_FUNCTION();
		// Render 2D 
		Camera* mainCamera = nullptr;
		glm::mat4* cameraTransform = nullptr;
		{
			auto view = m_Registry.view<TransformComponent, CameraComponent>();
			for (auto entity : view)
			{
				auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);

				if (camera.Primary)
				{
					mainCamera = &camera.Camera;
					cameraTransform = &transform.GetTransform();
					break;
				}
			}
		}

		if (mainCamera)
		{
			Renderer2D::BeginScene(mainCamera->GetProjection(), *cameraTransform);

            // Draw Sprites
            {
                auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
                for (auto entity : group)
                {
                    auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

                    Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
                }
            }
            // Draw Circles
            {
                auto view = m_Registry.view<TransformComponent, CircleRendererComponent>();
                for (auto entity : view)
                {
                    auto [transform, circle] = view.get<TransformComponent, CircleRendererComponent>(entity);
    
                    Renderer2D::DrawCircle(transform.GetTransform(), circle, (int)entity);
                }
            }
			Renderer2D::EndScene();
		}
    }

    void Scene::OnPauseStart()
	{
		m_IsPaused = true;
	}

	void Scene::OnPauseStop()
	{
		m_IsPaused = false;
	}


	void Scene::OnRuntimeStart()
	{
		m_IsRunning = true;

		OnScriptStart();

		OnPhysics2DStart();
			
	}

	void Scene::OnUpdateRuntime(Time time)
	{
		CE_PROFILE_FUNCTION();
        if (!m_IsPaused)
        {
            // Update Scripts
			{
				auto view = m_Registry.view<ScriptComponent>();
				for (auto e : view)
				{
					Entity entity = { e, this };
					ScriptEngine::OnUpdateEntity(entity, time.deltaTime());
				}
			}
			// Physics 2D

			{
				const int32_t velocityIterations = 6;
				const int32_t positionIterations = 2;

				m_PhysicsWorld->Step(time.deltaTime(), velocityIterations, positionIterations);

				// Retrieve transform from Box2D
				auto view = m_Registry.view<Rigidbody2DComponent>();
				for (auto e : view)
				{
					Entity entity{ e, this };
					auto& transform = entity.GetComponent<TransformComponent>();
					auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
                    
                    if (rb2d.JustAdded)
                        OnPhysics2DAdd();

					b2Body* body = (b2Body*)rb2d.RuntimeBody;
					const auto& position = body->GetPosition();
					transform.Position.x = position.x;
					transform.Position.y = position.y;
					transform.Rotation.z = body->GetAngle();
				}

			}
		}
		// Render 2D 
		Camera* mainCamera = nullptr;
		glm::mat4* cameraTransform = nullptr;
		{
			auto view = m_Registry.view<TransformComponent, CameraComponent>();
			for (auto entity : view)
			{
				auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);

				if (camera.Primary)
				{
					mainCamera = &camera.Camera;
					cameraTransform = &transform.GetTransform();
					break;
				}
			}
		}

		if (mainCamera)
		{
			Renderer2D::BeginScene(mainCamera->GetProjection(), *cameraTransform);
            // Draw Sprites
            {
                auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
                for (auto entity : group)
                {
                    auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

                    Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
                }
            }
            // Draw Circles
            {
                auto view = m_Registry.view<TransformComponent, CircleRendererComponent>();
                for (auto entity : view)
                {
                    auto [transform, circle] = view.get<TransformComponent, CircleRendererComponent>(entity);
    
                    Renderer2D::DrawCircle(transform.GetTransform(), circle, (int)entity);
                }
            }
			Renderer2D::EndScene();
		}
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

void Scene::OnRuntimeStop()
	{
		m_IsRunning = false;
        OnPhysics2DStop();
        ScriptEngine::OnRuntimeStop();
	}

	void Scene::OnSimulationStart()
	{
        OnPhysics2DStart();
	}

	void Scene::OnUpdateSimulation(Time time, EditorCamera& camera)
	{
		CE_PROFILE_FUNCTION();
        if (!m_IsPaused)
		{
			// Physics 2D

			{
				const int32_t velocityIterations = 6;
				const int32_t positionIterations = 2;

				m_PhysicsWorld->Step(time.deltaTime(), velocityIterations, positionIterations);

				// Retrieve transform from Box2D
				auto view = m_Registry.view<Rigidbody2DComponent>();
				for (auto e : view)
				{
					Entity entity{ e, this };
					auto& transform = entity.GetComponent<TransformComponent>();
					auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
                    if (rb2d.JustAdded)
                        OnPhysics2DAdd();

                    b2Body* body = (b2Body*)rb2d.RuntimeBody;
                    const auto& position = body->GetPosition();
                    transform.Position.x = position.x;
                    transform.Position.y = position.y;
                    transform.Rotation.z = body->GetAngle();
                }
			}
		}
		Renderer2D::BeginScene(camera);

		// Draw Sprites
		{
			auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto entity : group)
			{
				auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

				Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
			}
		}
		// Draw Circles
		{
			auto view = m_Registry.view<TransformComponent, CircleRendererComponent>();
			for (auto entity : view)
			{
				auto [transform, circle] = view.get<TransformComponent, CircleRendererComponent>(entity);

				Renderer2D::DrawCircle(transform.GetTransform(), circle, (int)entity);
			}
		}

		Renderer2D::EndScene();

	}

	void Scene::OnSimulationStop()
	{
		OnPhysics2DStop();
	}

	void Scene::OnPhysics2DStart()
	{
		CE_PROFILE_FUNCTION();

		m_PhysicsWorld = new b2World({ 0.0f, -9.8f });
        OnPhysics2DAdd();
    }

	void Scene::OnPhysics2DStop()
	{
		delete m_PhysicsWorld;
		m_PhysicsWorld = nullptr;
	}

    void Scene::OnPhysics2DAdd()
    {
		auto view = m_Registry.view<Rigidbody2DComponent>();
		for (auto e : view)
		{
			Entity entity = { e, this };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
            if (rb2d.JustAdded)
            {
                b2BodyDef bodyDef;
                bodyDef.type = Rigidbody2DTypeToBox2DType(rb2d.Type);
                bodyDef.position.Set(transform.Position.x, transform.Position.y);
                bodyDef.angle = transform.Rotation.z;

                b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
                body->SetFixedRotation(rb2d.FixedRotation);
                rb2d.RuntimeBody = body;
                rb2d.JustAdded = false;

                if (entity.HasComponent<BoxCollider2DComponent>())
                {
                    auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
    
                    b2PolygonShape boxShape;
                    boxShape.SetAsBox(transform.Scale.x * bc2d.Size.x, transform.Scale.y * bc2d.Size.y, { bc2d.Offset.x, bc2d.Offset.y }, glm::radians(bc2d.Rotation));

                    b2FixtureDef fixtureDef;
                    fixtureDef.shape = &boxShape;
                    fixtureDef.density = bc2d.Density;
                    fixtureDef.friction = bc2d.Friction;
                    fixtureDef.restitution = bc2d.Restitution;
                    fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;
                    body->CreateFixture(&fixtureDef);
                }
    
                if (entity.HasComponent<CircleCollider2DComponent>())
                {
                    auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();
    
                    b2CircleShape circleShape;
                    circleShape.m_p.Set(cc2d.Offset.x, cc2d.Offset.y);
                    circleShape.m_radius = transform.Scale.x * cc2d.Radius;

                    b2FixtureDef fixtureDef;
                    fixtureDef.shape = &circleShape;
                    fixtureDef.density = cc2d.Density;
                    fixtureDef.friction = cc2d.Friction;
                    fixtureDef.restitution = cc2d.Restitution;
                    fixtureDef.restitutionThreshold = cc2d.RestitutionThreshold;
                    body->CreateFixture(&fixtureDef);
                }
            }
        }
    }

    void Scene::OnScriptStart()
    {
        auto view = m_Registry.view<ScriptComponent>();
        for (auto& e : view)
        {
            Entity entity = { e, this };
			ScriptEngine::OnStartEntity(entity);
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
        CE_API_INFO("Created Entity with UUID : {}", static_cast<uint64_t>(uuid));
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
		m_EntityMap.erase(entity.GetUUID());
		m_Registry.destroy(entity);
	}


	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
	}
	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
			component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}


}
