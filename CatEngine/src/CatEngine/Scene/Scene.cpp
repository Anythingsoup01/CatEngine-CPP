#include "Scene.h"
#include "CatEngine/Core/Log.h"
#include "cepch.h"

#include "Components/Components.h"
#include "Components/Required/RelationshipComponent.h"

#include "Entity.h"

#include "CatEngine/Renderer/Renderer2D.h"

#include <cstdint>
#include <glm/glm.hpp>

#include <box2d/b2_body.h>
#include <box2d/b2_circle_shape.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_world.h>
#include <random>

#include "CatEngine/Scripting/ScriptEngine.h"

namespace CatEngine {
static b2BodyType
Rigidbody2DTypeToBox2DType(Rigidbody2DComponent::BodyType bodyType) {
  switch (bodyType) {
  case CatEngine::Rigidbody2DComponent::BodyType::Static:
    return b2_staticBody;
  case CatEngine::Rigidbody2DComponent::BodyType::Kinematic:
    return b2_kinematicBody;
  case CatEngine::Rigidbody2DComponent::BodyType::Dynamic:
    return b2_dynamicBody;
  }

  CE_API_ASSERT(false, "Unknown body type!");
  return b2_staticBody;
}

Scene::Scene() {}

Scene::~Scene() { ShutdownRuntime(); }

template <typename T>
static void
CopyComponent(entt::registry &dstRegistry, entt::registry &srcRegistry,
              const std::unordered_map<UUID, entt::entity> &enttMap) {
  auto srcEntities = srcRegistry.view<T>();
  for (auto srcEntity : srcEntities) {
    entt::entity destEntity =
        enttMap.at(srcRegistry.get<IDComponent>(srcEntity).ID);

    auto &srcComponent = srcRegistry.get<T>(srcEntity);
    auto &destComponent =
        dstRegistry.emplace_or_replace<T>(destEntity, srcComponent);
  }
}

template <typename... Component>
static void CopyComponentIfExists(Entity dst, Entity src) {
  (
      [&]() {
        if (src.HasComponent<Component>())
          dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
      }(),
      ...);
}
template <typename... Component>
static void CopyComponentIfExists(ComponentGroup<Component...>, Entity dst,
                                  Entity src) {
  CopyComponentIfExists<Component...>(dst, src);
}

void Scene::CopyTo(Ref<Scene> &target) {
  CE_PROFILE_FUNCTION();

  target->m_ViewportWidth = m_ViewportWidth;
  target->m_ViewportHeight = m_ViewportHeight;

  auto &srcRegistry = m_Registry;
  auto &dstRegistry = target->m_Registry;

  std::unordered_map<UUID, entt::entity> enttMap;

  // 1) Create empty entities with the same UUID
  auto idView = srcRegistry.view<IDComponent>();
  for (auto e : idView) {
    UUID uuid = srcRegistry.get<IDComponent>(e).ID;
    auto name = srcRegistry.get<NameComponent>(e).Name;
    UUID parent = srcRegistry.get<RelationshipComponent>(e).Parent;

    enttMap[uuid] = target->CreateEntityWithUUID(uuid, name, parent);
  }

  target->RegisterParents();

  CopyComponent<TagComponent>(dstRegistry, srcRegistry, enttMap);
  CopyComponent<LayerComponent>(dstRegistry, srcRegistry, enttMap);
  CopyComponent<TransformComponent>(dstRegistry, srcRegistry, enttMap);
  CopyComponent<CameraComponent>(dstRegistry, srcRegistry, enttMap);
  CopyComponent<SpriteRendererComponent>(dstRegistry, srcRegistry, enttMap);
  CopyComponent<CircleRendererComponent>(dstRegistry, srcRegistry, enttMap);
  CopyComponent<BoxCollider2DComponent>(dstRegistry, srcRegistry, enttMap);
  CopyComponent<CircleCollider2DComponent>(dstRegistry, srcRegistry, enttMap);
  CopyComponent<Rigidbody2DComponent>(dstRegistry, srcRegistry, enttMap);
  CopyComponent<ScriptComponent>(dstRegistry, srcRegistry, enttMap);
}

void Scene::OnUpdateEditor(Time ts, EditorCamera &camera) {
  CE_PROFILE_FUNCTION();
  UpdateTransforms();
  Renderer2D::BeginScene(camera);
  const auto &rootEntities = GetRootEntities();
  for (auto entityID : rootEntities) {
    Entity rootEntity = GetEntityByUUID(entityID);
    if (rootEntity) {
      // Pass identity matrix (1.0) as the base parent matrix for roots
      DrawEntityRecursive(rootEntity, glm::mat4(1.0f));
    }
  }
  Renderer2D::EndScene();
}

void Scene::OnUpdateMainCameraPreview(Time ts) {
  CE_PROFILE_FUNCTION();
  // Render 2D
  Camera *mainCamera = nullptr;
  glm::mat4 cameraTransform;
  {
    auto view = m_Registry.view<TransformComponent, CameraComponent>();
    for (auto entity : view) {
      auto [transform, camera] =
          view.get<TransformComponent, CameraComponent>(entity);

      if (camera.Primary) {
        mainCamera = &camera.Camera;
        cameraTransform = transform.GetWorldTransform();
        break;
      }
    }
  }

  if (mainCamera) {
    Renderer2D::BeginScene(mainCamera->GetProjection(), cameraTransform);
    const auto &rootEntities = GetRootEntities();
    for (auto entityID : rootEntities) {
      Entity rootEntity = GetEntityByUUID(entityID);
      if (rootEntity) {
        // Pass identity matrix (1.0) as the base parent matrix for roots
        DrawEntityRecursive(rootEntity, glm::mat4(1.0f));
      }
    }
    Renderer2D::EndScene();
  }
}

void Scene::OnPauseStart() { m_IsPaused = true; }

void Scene::OnPauseStop() { m_IsPaused = false; }

void Scene::OnRuntimeStart() {
  m_IsRunning = true;
  OnPhysics2DStart();
  OnScriptStart();
}

void Scene::OnUpdateRuntime(Time time) {
  CE_PROFILE_FUNCTION();
  UpdateTransforms();

  if (!m_IsPaused) {
    // Update Scripts
    {
      auto view = m_Registry.view<ScriptComponent>();
      for (auto e : view) {
        Entity entity = {e, this};
        ScriptEngine::GetMutable().OnUpdateEntity(entity, time.deltaTime());
      }
    }
    // Physics 2D
    OnPhysics2DUpdate(time);
  }

  // Render 2D
  Camera *mainCamera = nullptr;
  glm::mat4 cameraTransform;
  {
    auto view = m_Registry.view<TransformComponent, CameraComponent>();
    for (auto entity : view) {
      auto [transform, camera] =
          view.get<TransformComponent, CameraComponent>(entity);

      if (camera.Primary) {
        mainCamera = &camera.Camera;
        cameraTransform = transform.GetWorldTransform();
        break;
      }
    }
  }

  if (mainCamera) {
    Renderer2D::BeginScene(mainCamera->GetProjection(), cameraTransform);
    // Draw Sprites
    const auto &rootEntities = GetRootEntities();
    for (auto entityID : rootEntities) {
      Entity rootEntity = GetEntityByUUID(entityID);
      if (rootEntity) {
        // Pass identity matrix (1.0) as the base parent matrix for roots
        DrawEntityRecursive(rootEntity, glm::mat4(1.0f));
      }
    }
    Renderer2D::EndScene();
  }
}

void Scene::OnViewportResize(uint32_t width, uint32_t height) {
  if (m_ViewportWidth == width && m_ViewportHeight == height)
    return;

  m_ViewportWidth = width;
  m_ViewportHeight = height;

  // Resize non-fixed aspect ratios
  auto view = m_Registry.view<CameraComponent>();
  for (auto entity : view) {
    auto &cameraComponent = view.get<CameraComponent>(entity);
    if (!cameraComponent.FixedAspectRatio) {
      cameraComponent.Camera.SetViewportSize(width, height);
    }
  }
}

void Scene::ShutdownRuntime() {
  if (!m_IsRunning)
    return;
  m_IsRunning = false;
  ScriptEngine::GetMutable().OnRuntimeStop();

  // Destroy physics world safely
  if (m_PhysicsWorld) {
    m_PhysicsWorld->SetContactListener(nullptr);
    delete m_PhysicsWorld;
    m_PhysicsWorld = nullptr;
  }

  // Clear any maps that reference entities
  m_EntityMap.clear();

  // Finally clear the registry
  m_Registry.clear();
}

void Scene::OnRuntimeStop() {
  m_IsRunning = false;
  OnPhysics2DStop();
  ScriptEngine::GetMutable().OnRuntimeStop();
}

void Scene::OnSimulationStart() { OnPhysics2DStart(); }

void Scene::OnUpdateSimulation(Time time, EditorCamera &camera) {
  CE_PROFILE_FUNCTION();
  UpdateTransforms();
  if (!m_IsPaused) {
    // Physics 2D

    {
      const int32_t velocityIterations = 6;
      const int32_t positionIterations = 2;

      m_PhysicsWorld->Step(time.deltaTime(), velocityIterations,
                           positionIterations);

      // Retrieve transform from Box2D
      auto view = m_Registry.view<Rigidbody2DComponent>();
      for (auto e : view) {
        Entity entity{e, this};
        auto &transform = entity.GetComponent<TransformComponent>();
        auto &rb2d = entity.GetComponent<Rigidbody2DComponent>();

        b2Body *body = (b2Body *)rb2d.RuntimeBody;
        const auto &position = body->GetPosition();
        transform.LocalPosition.x = position.x;
        transform.LocalPosition.y = position.y;
        transform.LocalRotation.z = body->GetAngle();
      }
    }
  }
  Renderer2D::BeginScene(camera);
  const auto &rootEntities = GetRootEntities();
  for (auto entityID : rootEntities) {
    Entity rootEntity = GetEntityByUUID(entityID);
    if (rootEntity) {
      // Pass identity matrix (1.0) as the base parent matrix for roots
      DrawEntityRecursive(rootEntity, glm::mat4(1.0f));
    }
  }
  Renderer2D::EndScene();
}

void Scene::OnSimulationStop() { OnPhysics2DStop(); }

void Scene::OnPhysics2DStart() {
  CE_PROFILE_FUNCTION();

  UpdateTransforms(); 

  m_PhysicsWorld = new b2World({0.0f, -9.8f});
  m_PhysicsWorld->SetContactListener(&m_ContactListenter);
  
  auto view = m_Registry.view<Rigidbody2DComponent>();
  for (auto e : view) {
    Entity entity = {e, this};
    auto &rb2d = entity.GetComponent<Rigidbody2DComponent>();

    // 1. Compute true absolute world transform recursively
    glm::mat4 worldTransform = entity.GetWorldTransform();
    glm::vec3 worldPos, worldRot, worldScale;
    Math::DecomposeTransform(worldTransform, worldPos, worldRot, worldScale);

    b2BodyDef bodyDef;
    bodyDef.type = Rigidbody2DTypeToBox2DType(rb2d.Type);
    bodyDef.position.Set(worldPos.x, worldPos.y); // Use true world position
    bodyDef.angle = worldRot.z;                   // Use true world rotation

    b2Body *body = m_PhysicsWorld->CreateBody(&bodyDef);
    body->SetFixedRotation(rb2d.FixedRotation);
    rb2d.RuntimeBody = body;

    if (entity.HasComponent<BoxCollider2DComponent>()) {
      auto &bc2d = entity.GetComponent<BoxCollider2DComponent>();

      b2PolygonShape boxShape;
      // Use decomposed worldScale instead of cached WorldScale
      boxShape.SetAsBox(worldScale.x * bc2d.Size.x,
                        worldScale.y * bc2d.Size.y,
                        {bc2d.Offset.x, bc2d.Offset.y},
                        glm::radians(bc2d.Rotation));

      b2FixtureDef fixtureDef;
      fixtureDef.shape = &boxShape;
      fixtureDef.density = bc2d.Density;
      fixtureDef.friction = bc2d.Friction;
      fixtureDef.restitution = bc2d.Restitution;
      fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;
      fixtureDef.userData.pointer = static_cast<uintptr_t>(entity.GetUUID());
      body->CreateFixture(&fixtureDef);
    }

    if (entity.HasComponent<CircleCollider2DComponent>()) {
      auto &cc2d = entity.GetComponent<CircleCollider2DComponent>();

      b2CircleShape circleShape;
      circleShape.m_p.Set(cc2d.Offset.x, cc2d.Offset.y);
      // Use decomposed worldScale
      circleShape.m_radius = worldScale.x * cc2d.Radius;

      b2FixtureDef fixtureDef;
      fixtureDef.shape = &circleShape;
      fixtureDef.density = cc2d.Density;
      fixtureDef.friction = cc2d.Friction;
      fixtureDef.restitution = cc2d.Restitution;
      fixtureDef.restitutionThreshold = cc2d.RestitutionThreshold;
      fixtureDef.userData.pointer = static_cast<uintptr_t>(entity.GetUUID());
      body->CreateFixture(&fixtureDef);
    }
  }
}

void Scene::OnPhysics2DUpdate(Time ts) {
  const int32_t velocityIterations = 6;
  const int32_t positionIterations = 6;

  m_PhysicsWorld->Step(ts.deltaTime(), velocityIterations, positionIterations);

  // Retrieve transform from Box2D
  auto view = m_Registry.view<Rigidbody2DComponent>();
  for (auto e : view) {
    Entity entity{e, this};
    auto &transform = entity.GetComponent<TransformComponent>();
    auto &rb2d = entity.GetComponent<Rigidbody2DComponent>();
    auto &rc = entity.GetComponent<RelationshipComponent>();

    b2Body *body = (b2Body *)rb2d.RuntimeBody;
    if (!body) continue;

    const auto &position = body->GetPosition();
    float angle = body->GetAngle();

    if (rc.Parent != 0) {
      // --- CHILD ENTITY ---
      Entity parent = GetEntityByUUID(rc.Parent);
      if (parent) {
        // 1. Build the body's world matrix from Box2D data
        glm::mat4 bodyWorldMatrix = 
            glm::translate(glm::mat4(1.0f), {position.x, position.y, 0.0f}) *
            glm::rotate(glm::mat4(1.0f), angle, {0.0f, 0.0f, 1.0f}) *
            glm::scale(glm::mat4(1.0f), transform.LocalScale);

        // 2. Convert world matrix to local space relative to the parent
        glm::mat4 parentWorldMatrix = parent.GetWorldTransform();
        glm::mat4 localMatrix = glm::inverse(parentWorldMatrix) * bodyWorldMatrix;

        // 3. Decompose local matrix
        glm::vec3 translation, rotation, scale;
        Math::DecomposeTransform(localMatrix, translation, rotation, scale);

        // 4. Assign back using your LocalPosition.z 2D angle convention
        transform.LocalPosition.x = translation.x;
        transform.LocalPosition.y = translation.y;
        transform.LocalPosition.z = rotation.z; // Local 2D rotation angle
        transform.LocalScale = scale;
      }
    } else {
      // --- ROOT ENTITY ---
      transform.LocalPosition.x = position.x;
      transform.LocalPosition.y = position.y;
      transform.LocalPosition.z = angle;
    }
  }
}

void Scene::OnPhysics2DStop() {
  m_PhysicsWorld->SetContactListener(nullptr);
  delete m_PhysicsWorld;
  m_PhysicsWorld = nullptr;
}

void Scene::OnScriptStart() {
  auto view = m_Registry.view<ScriptComponent>();
  for (auto &e : view) {
    Entity entity = {e, this};
    ScriptEngine::GetMutable().OnStartEntity(entity);
  }
}

void Scene::UpdateTransforms() {
  for (auto entityID : m_RootEntities) {
    Entity rootEntity = GetEntityByUUID(entityID);
    // Pass an identity matrix as the "parent" matrix for roots
    CalculateTransformRecursive(rootEntity, glm::mat4(1.0f));
  }
}

void Scene::CalculateTransformRecursive(Entity entity,
                                        const glm::mat4 &parentWorldMatrix) {
  auto &tc = entity.GetComponent<TransformComponent>();
  auto &rc = entity.GetComponent<RelationshipComponent>();

  // 1. Calculate true world matrix via matrix multiplication
  glm::mat4 worldMatrix = parentWorldMatrix * tc.GetLocalTransform();

  // 2. Decompose back into World fields for rendering and physics
  Math::DecomposeTransform(worldMatrix, tc.WorldPosition, tc.WorldRotation,
                           tc.WorldScale);

  // 3. Pass this world matrix down to all children
  for (auto childUUID : rc.Children) {
    Entity childEntity = GetEntityByUUID(childUUID);
    if (childEntity) {
      CalculateTransformRecursive(childEntity, worldMatrix);
    }
  }
}

void Scene::DuplicateEntity(Entity entity) { PasteEntity(entity); }

Entity Scene::PasteEntity(Entity entity) {
  CE_PROFILE_FUNCTION();

  Entity newEntity = CreateEntity(entity.GetName() + "(Copy)");
  CopyComponentIfExists(AllComponents{}, newEntity, entity);
  return newEntity;
}
Entity Scene::FindEntityByName(std::string_view name) {
  auto view = m_Registry.view<NameComponent>();
  for (auto entity : view) {
    const NameComponent &nameComponent = view.get<NameComponent>(entity);
    if (nameComponent.Name == name) {
      return Entity{entity, this};
    }
  }
  return {};
}
Entity Scene::GetEntityByUUID(UUID entityID) {
  if (m_EntityMap.find(entityID) != m_EntityMap.end())
    return {m_EntityMap.at(entityID), this};
  return {};
}
Entity Scene::GetPrimaryCameraEntity() {
  CE_PROFILE_FUNCTION();

  auto view = m_Registry.view<CameraComponent>();
  for (auto entity : view) {
    const auto &cameraComponent = view.get<CameraComponent>(entity);
    if (cameraComponent.Primary)
      return Entity{entity, this};
  }
  return {};
}
Entity Scene::CreateEntity(const std::string &name) {
  return CreateEntityWithUUID(UUID(), name);
}
Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string &name,
                                   UUID parent) {
  CE_PROFILE_FUNCTION();
  Entity entity = {m_Registry.create(), this};

  entity.AddComponent<IDComponent>(uuid);
  auto &tag = entity.AddComponent<TagComponent>();
  tag.Tag = "None";
  auto &layer = entity.AddComponent<LayerComponent>();
  layer.Layer = "Default";
  auto &entityName = entity.AddComponent<NameComponent>();
  entityName.Name = name.empty() ? "GameObject" : name;
  auto &relationship = entity.AddComponent<RelationshipComponent>();
  if (parent.uuid() != 0) { // Child
    relationship.Parent = parent;
  } else {
    m_RootEntities.push_back(uuid);
  }

  entity.AddComponent<TransformComponent>();

  m_EntityMap[uuid] = entity.GetEntityID();

  return entity;
}

void Scene::DeleteEntity(Entity entity) {
  auto &rc = entity.GetComponent<RelationshipComponent>();
  for (auto &child : rc.Children) {
    DeleteEntity(GetEntityByUUID(child));
  }
  m_EntityMap.erase(entity.GetUUID());
  m_RootEntities.erase(std::remove(m_RootEntities.begin(),
                                     m_RootEntities.end(),
                                     entity.GetUUID()),
                         m_RootEntities.end());
  m_Registry.destroy(entity);

}

void Scene::DrawEntityRecursive(Entity entity,
                                const glm::mat4 &parentWorldMatrix) {
  auto &tc = entity.GetComponent<TransformComponent>();
  auto &rc = entity.GetComponent<RelationshipComponent>();

  // 1. Compute true world matrix by multiplying parent's matrix with local
  // matrix
  glm::mat4 worldMatrix = parentWorldMatrix * tc.GetLocalTransform();

  // 2. (Optional) Update cached world components if physics or other systems
  // need them
  Math::DecomposeTransform(worldMatrix, tc.WorldPosition, tc.WorldRotation,
                           tc.WorldScale);

  // 3. Render Sprites
  if (entity.HasComponent<SpriteRendererComponent>()) {
    auto &sprite = entity.GetComponent<SpriteRendererComponent>();
    Renderer2D::DrawSprite(worldMatrix, sprite, (int)entity.GetEntityID());
  }

  // 4. Render Circles
  if (entity.HasComponent<CircleRendererComponent>()) {
    auto &circle = entity.GetComponent<CircleRendererComponent>();
    Renderer2D::DrawCircle(worldMatrix, circle, (int)entity.GetEntityID());
  }

  // 5. Recursively draw all children, passing this entity's world matrix down
  for (auto childUUID : rc.Children) {
    Entity child = GetEntityByUUID(childUUID);
    if (child) {
      DrawEntityRecursive(child, worldMatrix);
    }
  }
}

void Scene::ReparentEntity(Entity entity, Entity newParent) {
  auto &rc = entity.GetComponent<RelationshipComponent>();
  auto &tc = entity.GetComponent<TransformComponent>();

  glm::mat4 oldWorldTransform = entity.GetWorldTransform();

  // 1. Detach from old parent or root list
  if (rc.Parent != 0) {
    Entity oldParent = GetEntityByUUID(rc.Parent);
    auto &oldChildren =
        oldParent.GetComponent<RelationshipComponent>().Children;
    oldChildren.erase(
        std::remove(oldChildren.begin(), oldChildren.end(), entity.GetUUID()),
        oldChildren.end());
  } else {
    for (auto child : rc.Children) {
      if (child.uuid() == newParent.GetUUID()) {
        return;
      }
    }
    // Remove from root entities list if you track root entities separately
    m_RootEntities.erase(std::remove(m_RootEntities.begin(),
                                     m_RootEntities.end(), entity.GetUUID()),
                         m_RootEntities.end());
  }

  

  // 2. Attach to new parent
  if (newParent) {
    rc.Parent = newParent.GetUUID();
    auto &newParentRc = newParent.GetComponent<RelationshipComponent>();
    newParentRc.Children.push_back(entity.GetUUID());

    // 4. Calculate new local transform: Local = Inverse(NewParentWorld) * OldWorld
    glm::mat4 newParentWorldMatrix = newParent.GetWorldTransform();
    glm::mat4 localMatrix = glm::inverse(newParentWorldMatrix) * oldWorldTransform;

    // 5. Decompose into the component's local fields
    glm::vec3 localRotation;
    Math::DecomposeTransform(localMatrix, tc.LocalPosition, localRotation, tc.LocalScale);
    tc.LocalRotation = localRotation;
    } else {
    // Unparenting to root (Parent == 0)
    rc.Parent = 0;

    // When becoming a root, local transform IS the world transform
    glm::vec3 localRotation;
    Math::DecomposeTransform(oldWorldTransform, tc.LocalPosition, localRotation, tc.LocalScale);
    tc.LocalRotation = localRotation;

    // Make sure to add it back to your root entities list if you maintain one
    m_RootEntities.push_back(entity.GetUUID());
  }
}

void Scene::MoveEntityBefore(Entity entityToMove, Entity targetEntity) {
  auto &moveRel = entityToMove.GetComponent<RelationshipComponent>();
  auto &targetRel = targetEntity.GetComponent<RelationshipComponent>();

  // Unlink from current parent/root
  if (moveRel.Parent != 0) {
    Entity oldParent = GetEntityByUUID(moveRel.Parent);
    auto &oldChildren =
        oldParent.GetComponent<RelationshipComponent>().Children;
    oldChildren.erase(std::remove(oldChildren.begin(), oldChildren.end(),
                                  entityToMove.GetUUID()),
                      oldChildren.end());
  } else {
    m_RootEntities.erase(std::remove(m_RootEntities.begin(),
                                     m_RootEntities.end(),
                                     entityToMove.GetUUID()),
                         m_RootEntities.end());
  }

  // Match target's parent
  moveRel.Parent = targetRel.Parent;

  if (targetRel.Parent != 0) {
    Entity parent = GetEntityByUUID(targetRel.Parent);
    auto &siblings = parent.GetComponent<RelationshipComponent>().Children;
    auto it =
        std::find(siblings.begin(), siblings.end(), targetEntity.GetUUID());
    siblings.insert(it, entityToMove.GetUUID());
  } else {
    auto it = std::find(m_RootEntities.begin(), m_RootEntities.end(),
                        targetEntity.GetUUID());
    m_RootEntities.insert(it, entityToMove.GetUUID());
  }
}

void Scene::MoveEntityAfter(Entity entityToMove, Entity targetEntity) {
  auto &moveRel = entityToMove.GetComponent<RelationshipComponent>();
  auto &targetRel = targetEntity.GetComponent<RelationshipComponent>();

  // Unlink from current parent/root
  if (moveRel.Parent != 0) {
    Entity oldParent = GetEntityByUUID(moveRel.Parent);
    auto &oldChildren =
        oldParent.GetComponent<RelationshipComponent>().Children;
    oldChildren.erase(std::remove(oldChildren.begin(), oldChildren.end(),
                                  entityToMove.GetUUID()),
                      oldChildren.end());
  } else {
    m_RootEntities.erase(std::remove(m_RootEntities.begin(),
                                     m_RootEntities.end(),
                                     entityToMove.GetUUID()),
                         m_RootEntities.end());
  }

  // Match target's parent
  moveRel.Parent = targetRel.Parent;

  if (targetRel.Parent != 0) {
    Entity parent = GetEntityByUUID(targetRel.Parent);
    auto &siblings = parent.GetComponent<RelationshipComponent>().Children;
    auto it =
        std::find(siblings.begin(), siblings.end(), targetEntity.GetUUID());
    if (it != siblings.end()) {
      siblings.insert(it + 1, entityToMove.GetUUID());
    }
  } else {
    auto it = std::find(m_RootEntities.begin(), m_RootEntities.end(),
                        targetEntity.GetUUID());
    if (it != m_RootEntities.end()) {
      m_RootEntities.insert(it + 1, entityToMove.GetUUID());
    }
  }
}

void Scene::RegisterParents() {
  auto view = m_Registry.view<IDComponent, RelationshipComponent>();
  for (auto entity : view) {
    auto [id, relation] = view.get<IDComponent, RelationshipComponent>(entity);

    if (relation.Parent != 0) {
      Entity parent = GetEntityByUUID(relation.Parent);
      parent.GetComponent<RelationshipComponent>().Children.push_back(id.ID);
    }
  }
}

template <typename T>
void Scene::OnComponentAdded(Entity entity, T &component) {
  static_assert(false, "Component Not Supported!");
}
template <>
void Scene::OnComponentAdded<LayerComponent>(Entity entity,
                                             LayerComponent &component) {}
template <>
void Scene::OnComponentAdded<TagComponent>(Entity entity,
                                           TagComponent &component) {}
template <>
void Scene::OnComponentAdded<CameraComponent>(Entity entity,
                                              CameraComponent &component) {
  if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
    component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
}
template <>
void Scene::OnComponentAdded<TransformComponent>(
    Entity entity, TransformComponent &component) {}
template <>
void Scene::OnComponentAdded<CircleRendererComponent>(
    Entity entity, CircleRendererComponent &component) {}

template <>
void Scene::OnComponentAdded<SpriteRendererComponent>(
    Entity entity, SpriteRendererComponent &component) {}
template <>
void Scene::OnComponentAdded<Rigidbody2DComponent>(
    Entity entity, Rigidbody2DComponent &component) {}

template <>
void Scene::OnComponentAdded<NameComponent>(Entity entity,
                                            NameComponent &component) {}
template <>
void Scene::OnComponentAdded<BoxCollider2DComponent>(
    Entity entity, BoxCollider2DComponent &component) {}
template <>
void Scene::OnComponentAdded<CircleCollider2DComponent>(
    Entity entity, CircleCollider2DComponent &component) {}
template <>
void Scene::OnComponentAdded<ScriptComponent>(Entity entity,
                                              ScriptComponent &component) {
  component.ScriptID = UUID();
}
template <>
void Scene::OnComponentAdded<IDComponent>(Entity entity,
                                          IDComponent &component) {}

template <>
void Scene::OnComponentAdded<RelationshipComponent>(
    Entity entity, RelationshipComponent &component) {}

} // namespace CatEngine
