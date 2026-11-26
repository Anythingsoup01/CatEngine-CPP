#include "cepch.h" // IWYU pragma: keep
#include "Scene.h"

#include "Components/Components.h" // IWYU pragma: keep

#include "Entity.h"

#include "CatEngine/AssetManager/AssetManager.h"
#include "CatEngine/Editor/PanelManager.h"
#include "CatEngine/Editor/Panels/ConsolePanel.h"
#include "CatEngine/Physics/Physics2D.h"
#include "CatEngine/Prefab/Prefab.h"
#include "CatEngine/Prefab/PrefabManager.h"
#include "CatEngine/Project/Project.h"
#include "CatEngine/Renderer/Renderer2D.h"
#include "CatEngine/Scripting/ScriptEngine.h"

namespace CatEngine {

Scene::Scene() {}

Scene::~Scene() { ShutdownRuntime(); }

template <typename T>
static void CopyComponent(entt::registry &dstRegistry, entt::registry &srcRegistry,
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

template <typename T>
static void CopyComponentIfExistsForEntity(entt::registry &dstRegistry,
                                           entt::registry &srcRegistry,
                                           entt::entity srcEntity,
                                           entt::entity destEntity) {
  if (srcRegistry.any_of<T>(srcEntity)) {
    auto &srcComponent = srcRegistry.get<T>(srcEntity);
    dstRegistry.emplace_or_replace<T>(destEntity, srcComponent);
  }
}

void Scene::RecursiveCopyTo(Ref<Scene> &target, const UUID &entityID) {
  if (!target)
    return;

  Entity srcEntityWrapper = GetEntityByUUID(entityID);
  UUID uuid = srcEntityWrapper.GetUUID();
  std::string name = srcEntityWrapper.GetName();
  UUID parent = srcEntityWrapper.GetComponent<RelationshipComponent>().Parent;
  Entity destEntityWrapper = target->CreateEntityWithUUID(uuid, name, parent);

  if (!destEntityWrapper)
    return;

  CopyComponentIfExists<
    TagComponent,
    LayerComponent,
    ObjectStateComponent,
    TransformComponent,
    CameraComponent,
    SpriteRendererComponent,
    CircleRendererComponent,
    Rigidbody2DComponent,
    BoxCollider2DComponent,
    CircleCollider2DComponent,
    ScriptComponentContainer,
    PrefabLinkComponent
    >(destEntityWrapper, srcEntityWrapper);

  auto &rc = srcEntityWrapper.GetComponent<RelationshipComponent>();
  for (auto &child : rc.Children) {
    RecursiveCopyTo(target, child);
  }
}

void Scene::CopyTo(Ref<Scene> &target) {
  CE_PROFILE_FUNCTION();

  target->m_ViewportWidth = m_ViewportWidth;
  target->m_ViewportHeight = m_ViewportHeight;
  target->m_Handle = m_Handle;

  for (auto &parent : m_RootEntities) {
    RecursiveCopyTo(target, parent);
  }

  target->RegisterParents();
  target->UpdateAllPrefabs();
}

void Scene::OnUpdateEditor(Time ts, EditorCamera &camera) {
  CE_PROFILE_FUNCTION();

  if (!m_IsRunning) {
    ExecuteSceneCommandQueue();
    DeleteEntities();
  }

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
    auto view = m_Registry.view<CameraComponent>();
    for (auto entity : view) {
      Entity e{entity, this};
      auto &camera = view.get<CameraComponent>(entity);

      if (camera.Primary) {
        mainCamera = &camera.Camera;
        cameraTransform = e.GetWorldTransform();
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
  ExecuteSceneCommandQueue();

  float frameTime = time.deltaTime();

  if (frameTime > 0.25f)
    frameTime = 0.25f;

  if (!m_IsPaused) {
    m_Accumulator += frameTime;

    while (m_Accumulator >= m_FixedTimeStep) {
      // Loads the delete command to run the next update frame, allowing the physics engine to catch up
      DeleteEntities();

      // Update Scripts
      ScriptEngine::GetMutable().OnFixedUpdate(Time(m_FixedTimeStep));
      // Physics 2D
      OnPhysics2DUpdate(Time(m_FixedTimeStep));

      m_Accumulator -= m_FixedTimeStep;
    }

    ScriptEngine::GetMutable().OnUpdate(time.deltaTime());
  }

  UpdateTransforms();


  // Render 2D
  Camera *mainCamera = nullptr;
  glm::mat4 cameraTransform;
  {
    auto view = m_Registry.view<TransformComponent, CameraComponent>();
    for (auto entity : view) {
      auto [transform, camera] =
          view.get<TransformComponent, CameraComponent>(entity);

      Entity e = {entity, this};

      if (camera.Primary) {
        mainCamera = &camera.Camera;
        cameraTransform = e.GetWorldTransform();
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
  if (!m_IsRunning) {
    return;
  }
  m_IsRunning = false;
  ScriptEngine::GetMutable().OnRuntimeStop();

  Physics2D::Get().Stop();

  // Clear any maps that reference entities
  m_EntityMap.clear();

  // Finally clear the registry
  m_Registry.clear();
}

void Scene::OnRuntimeStop() {
  m_IsRunning = false;
  OnPhysics2DStop();
  ScriptEngine::GetMutable().OnRuntimeStop();
  
  auto view = m_Registry.view<ScriptComponentContainer>();
  for (auto entityID : view) {
    auto &container = view.get<ScriptComponentContainer>(entityID);
    for (auto &sc : container.Scripts) {
      sc.Loaded = false;
    }
  }
}

void Scene::OnSimulationStart() { OnPhysics2DStart(); }

void Scene::OnUpdateSimulation(Time time, EditorCamera &camera) {
  CE_PROFILE_FUNCTION();
  ExecuteSceneCommandQueue();
  UpdateTransforms();
  if (!m_IsPaused) {
    // Physics 2D
    Physics2D::Get().Update(time);
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

  // Render 2D
  Camera *mainCamera = nullptr;
  glm::mat4 cameraTransform;
  {
    auto view = m_Registry.view<TransformComponent, CameraComponent>();
    for (auto entity : view) {
      auto [transform, camera] =
          view.get<TransformComponent, CameraComponent>(entity);

      Entity e = {entity, this};

      if (camera.Primary) {
        mainCamera = &camera.Camera;
        cameraTransform = e.GetWorldTransform();
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

void Scene::OnSimulationStop() { OnPhysics2DStop(); }

void Scene::OnPhysics2DStart() {
  CE_PROFILE_FUNCTION();

  UpdateTransforms();

  Physics2D::Get().Start(this);
}

void Scene::OnPhysics2DUpdate(Time ts) { Physics2D::Get().Update(ts); }

void Scene::OnPhysics2DStop() { Physics2D::Get().Stop(); }

void Scene::OnScriptStart() { ScriptEngine::GetMutable().LoadRuntimeScripts(); }

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

void Scene::DeleteEntities() {
  m_SceneCommandQueue.push_back([this](){
    for (auto &entityID : m_DeletedEntities) {
      Entity e = this->GetEntityByUUID(entityID);
      if (!e)
        continue;
      m_EntityMap.erase(entityID);
      m_RootEntities.erase(std::remove(m_RootEntities.begin(), m_RootEntities.end(),
                                   entityID),
                       m_RootEntities.end());
      m_Registry.destroy(e);
    }
    m_DeletedEntities.clear();
  });
}

void Scene::DuplicateEntity(Entity entity) { PasteEntity(entity); }

Entity Scene::PasteEntity(Entity entity) {
  CE_PROFILE_FUNCTION();

  Entity newEntity = CreateEntity(entity.GetName() + "(Copy)");
  CopyComponentIfExists(AllComponents{}, newEntity, entity);
  return newEntity;
}
Entity Scene::FindEntityByName(const std::string_view &name) {
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
Entity Scene::CreateEntity(const std::string &name, UUID parent) {
  return CreateEntityWithUUID(UUID(), name, parent);
}
Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string &name, UUID parent) {
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
  auto &osc = entity.AddComponent<ObjectStateComponent>();
  if (parent.uuid() != 0) { // Child
    relationship.Parent = parent;
    UUID rootParentID = relationship.Parent;
    Entity parent = GetEntityByUUID(rootParentID);
    while (rootParentID != 0) {
      parent = GetEntityByUUID(rootParentID);
      auto &prc = parent.GetComponent<RelationshipComponent>();
      rootParentID = prc.Parent;
    }

    if (parent.HasComponent<PrefabLinkComponent>()) {
      osc.PrefabAdded = true;
    }
  } else {
    m_RootEntities.push_back(uuid);
  }

  entity.AddComponent<TransformComponent>();

  m_EntityMap[uuid] = entity.GetEntityID();

  return entity;
}

void Scene::DeleteEntity(Entity entity) {
  m_SceneCommandQueue.push_back([this, entity](){
    auto &rc = entity.GetComponent<RelationshipComponent>();
    UUID rootParentID = rc.Parent;
    Entity parent = GetEntityByUUID(rootParentID);
    while (rootParentID != 0) {
      parent = GetEntityByUUID(rootParentID);
      auto &prc = parent.GetComponent<RelationshipComponent>();
      rootParentID = prc.Parent;
    }

    if (parent.HasComponent<PrefabLinkComponent>()) {
      auto &plc = parent.GetComponent<PrefabLinkComponent>();
      std::string name = entity.GetComponent<NameComponent>().Name;
      plc.DeletedEntities.push_back(name);
    }

    for (auto &child : rc.Children) {
      DeleteEntity(GetEntityByUUID(child));
    }

    if (m_IsRunning) {
      if (entity.HasComponent<BoxCollider2DComponent>() || entity.HasComponent<CircleCollider2DComponent>()) {
        Physics2D::Get().EnableCollider(entity, false);
      }

      if (entity.HasComponent<Rigidbody2DComponent>()) {
        Physics2D::Get().EnableRigidbody(entity, false);
      }

      if (entity.HasComponent<ScriptComponentContainer>()) {
        ScriptEngine::GetMutable().DeleteEntity(entity);
      }
    }


    m_DeletedEntities.push_back(entity.GetUUID());
  });
}

void Scene::SetEntityLocked(Entity entity, bool locked) {
  auto& state = entity.GetComponent<ObjectStateComponent>();
  state.SelfLocked = locked;

  // Evaluate against parent hierarchy
  bool parentActive = true;
  auto& rel = entity.GetComponent<RelationshipComponent>();
  if (rel.Parent != 0) {
    Entity parent = GetEntityByUUID(rel.Parent);
    if (parent) {
      parentActive = parent.GetComponent<ObjectStateComponent>().HierarchyLocked
        && parent.GetComponent<ObjectStateComponent>().Locked;
    }
  }

  state.Locked = state.SelfLocked && state.HierarchyLocked;

  // Propagate down to all children immediately
  for (UUID childUUID : rel.Children) {
    Entity child = GetEntityByUUID(childUUID);
    if (child) {
      SetEntityChildLockedRecursive(child, state.Locked);
    }
  }
}

void Scene::SetEntityChildLockedRecursive(Entity entity, bool parentLocked) {
  auto& state = entity.GetComponent<ObjectStateComponent>();
  state.HierarchyLocked = parentLocked;
  bool effectiveLocked = state.SelfLocked && state.HierarchyLocked;

  auto& rel = entity.GetComponent<RelationshipComponent>();
  for (UUID childUUID : rel.Children) {
    Entity c = GetEntityByUUID(childUUID);
    if (c) {
      SetEntityChildLockedRecursive(c, effectiveLocked);
    }
  }
}

void Scene::SetEntityActive(Entity entity, bool active) {
  m_SceneCommandQueue.push_back([this, entity, active]() {
    auto& state = entity.GetComponent<ObjectStateComponent>();
    state.SelfActive = active;

    // Evaluate against parent hierarchy
    bool parentActive = true;
    auto& rel = entity.GetComponent<RelationshipComponent>();
    if (rel.Parent != 0) {
      Entity parent = GetEntityByUUID(rel.Parent);
      if (parent) {
        parentActive = parent.GetComponent<ObjectStateComponent>().HierarchyActive 
          && parent.GetComponent<ObjectStateComponent>().Active;
      }
    }

    state.HierarchyActive = parentActive;
    state.Active = state.SelfActive && state.HierarchyActive;

    if (entity.HasComponent<Rigidbody2DComponent>()) {
      if (!m_IsRunning)
        return;
      Physics2D::Get().EnableRigidbody(entity, state.Active);
    }

    if (entity.HasComponent<BoxCollider2DComponent>()) {
      if (!m_IsRunning)
        return;

      Physics2D::Get().EnableCollider(entity, state.Active);
    }

    if (entity.HasComponent<CircleCollider2DComponent>()) {
      if (!m_IsRunning)
        return;
      Physics2D::Get().EnableCollider(entity, state.Active);
    }

    if (entity.HasComponent<ScriptComponentContainer>()) {
      if (!m_IsRunning)
        return;
      ScriptEngine::GetMutable().OnStartEntity(entity);
    }


    // Propagate down to all children immediately
    for (UUID childUUID : rel.Children) {
      Entity child = GetEntityByUUID(childUUID);
      if (child) {
        SetEntityChildActiveRecursive(child, state.Active);
      }
    }
  });
}

void Scene::SetEntityChildActiveRecursive(Entity entity, bool parentActive) {
  auto& state = entity.GetComponent<ObjectStateComponent>();
  state.HierarchyActive = parentActive;
  bool effectiveActive = state.SelfActive && state.HierarchyActive;

  if (entity.HasComponent<Rigidbody2DComponent>()) {
    if (!m_IsRunning)
      return;
    Physics2D::Get().EnableRigidbody(entity, effectiveActive);
  }

  if (entity.HasComponent<BoxCollider2DComponent>()) {
    if (!m_IsRunning)
      return;
    Physics2D::Get().EnableCollider(entity, effectiveActive);
  }

  if (entity.HasComponent<CircleCollider2DComponent>()) {
    if (!m_IsRunning)
      return;
    Physics2D::Get().EnableCollider(entity, effectiveActive);
  }

  if (entity.HasComponent<ScriptComponentContainer>()) {
    if (!m_IsRunning)
      return;
    ScriptEngine::GetMutable().OnStartEntity(entity);
  }

  auto& rel = entity.GetComponent<RelationshipComponent>();
  for (UUID childUUID : rel.Children) {
    Entity c = GetEntityByUUID(childUUID);
    if (c) {
      SetEntityChildActiveRecursive(c, effectiveActive);
    }
  }
}

Entity Scene::InstantiatePrefab(const UUID &prefabID, const UUID &parentID) {
  CE_PROFILE_FUNCTION();

  // 1. Fetch the Prefab asset via AssetManager using its UUID/handle
  Ref<Prefab> prefab = AssetManager::GetAsset<Prefab>(prefabID);
  if (!prefab) {
    CE_API_WARN("Failed to load prefab with asset handle: {0}", (uint64_t)prefabID);
    auto consolePanel = PanelManager::Get().GetPanel<ConsolePanel>("ConsolePanel");
    consolePanel->WriteWarning("Failed to load prefab with asset handle: " + std::to_string(prefabID));
    return {};
  }

  Asset::MetaData metaData = Project::GetActive()->GetEditorAssetManager()->GetMetaData(prefabID);

  Ref<Scene> prefabScene = prefab->GetScene();
  auto &srcRegistry = prefabScene->GetReg();
  auto &dstRegistry = m_Registry;

  // 2. Validate prefab root entities
  const auto &prefabRoots = prefabScene->GetRootEntities();
  if (prefabRoots.empty()) {
    auto consolePanel = PanelManager::Get().GetPanel<ConsolePanel>("ConsolePanel");
    consolePanel->WriteWarning("Prefab: " + std::to_string(prefabID) + " has no root entity!");
    return {};
  }

  Entity prefabRootEntity = prefabScene->GetEntityByUUID(prefabRoots[0]);
  if (!prefabRootEntity)
    return {};

  UUID oldUUID = prefabRootEntity.GetUUID();
  UUID newUUID = UUID(); // Generate a fresh unique ID for this

  std::string name = prefabRootEntity.GetName() + " (Prefab)";
  Entity newEntity = CreateEntityWithUUID(newUUID, name, 0);
  newEntity.AddComponent<PrefabLinkComponent>(prefabID, prefabRootEntity.GetUUID(), metaData.LastUpdated);

  CopyComponentIfExists<
    TagComponent,
    LayerComponent,
    ObjectStateComponent,
    TransformComponent,
    CameraComponent,
    SpriteRendererComponent,
    CircleRendererComponent,
    Rigidbody2DComponent,
    BoxCollider2DComponent,
    CircleCollider2DComponent,
    ScriptComponentContainer
    >(newEntity, prefabRootEntity);


  // Recursive lambda to clone the prefab hierarchy
  std::function<void(Entity, UUID)> cloneEntityRecursive =
    [&](Entity srcEntity, UUID currentParentUUID) {
      UUID oldUUID = srcEntity.GetUUID();
      UUID newUUID = UUID(); // Generate a fresh unique ID for this
      // instantiation instance

      std::string name = srcEntity.GetName();
      Entity newEntity = CreateEntityWithUUID(newUUID, name, currentParentUUID);


      // Copy components from source prefab entity to new destination entity
      CopyComponentIfExists<
        TagComponent,
        LayerComponent,
        ObjectStateComponent,
        TransformComponent,
        CameraComponent,
        SpriteRendererComponent,
        CircleRendererComponent,
        Rigidbody2DComponent,
        BoxCollider2DComponent,
        CircleCollider2DComponent,
        ScriptComponentContainer,
        PrefabLinkComponent
        >(newEntity, srcEntity);

      // Recursively clone children
      auto &rc = srcEntity.GetComponent<RelationshipComponent>();
      for (UUID childOldUUID : rc.Children) {
        Entity childSrc = prefabScene->GetEntityByUUID(childOldUUID);
        if (childSrc) {
          cloneEntityRecursive(childSrc, newUUID);
        }
      }
    };

  auto &rc = prefabRootEntity.GetComponent<RelationshipComponent>();
  for (UUID childOldUUID : rc.Children) {
    Entity childSrc = prefabScene->GetEntityByUUID(childOldUUID);
    if (childSrc) {
      cloneEntityRecursive(childSrc, newUUID);
    }
  }

  // 4. If a valid parentID was provided, explicitly push the newly created root
  // into the parent's Children list
  if (parentID != 0) {
    Entity parentEntity = GetEntityByUUID(parentID);
    if (parentEntity && newEntity) {
      parentEntity.GetComponent<RelationshipComponent>().Children.push_back( newEntity.GetUUID());
    }
  }

  RegisterParents();
  return newEntity;
}



void Scene::DrawEntityRecursive(Entity entity,
                                const glm::mat4 &parentWorldMatrix) {
  auto &stateComponent = entity.GetComponent<ObjectStateComponent>();

  if (!stateComponent.Active) {
    return;
  }

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

  if (newParent && entity.IsChild(newParent.GetUUID())) {
    return;
  }

  // 1. Detach from old parent or root list
  if (rc.Parent != 0) {
    Entity oldParent = GetEntityByUUID(rc.Parent);
    auto &oldChildren =
        oldParent.GetComponent<RelationshipComponent>().Children;
    oldChildren.erase(
        std::remove(oldChildren.begin(), oldChildren.end(), entity.GetUUID()),
        oldChildren.end());
  } else {
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

    // 4. Calculate new local transform: Local = Inverse(NewParentWorld) *
    // OldWorld
    glm::mat4 newParentWorldMatrix = newParent.GetWorldTransform();
    glm::mat4 localMatrix =
        glm::inverse(newParentWorldMatrix) * oldWorldTransform;

    // 5. Decompose into the component's local fields
    glm::vec3 localRotation;
    Math::DecomposeTransform(localMatrix, tc.LocalPosition, localRotation,
                             tc.LocalScale);
    tc.LocalRotation = localRotation;
  } else {
    // Unparenting to root (Parent == 0)
    rc.Parent = 0;

    // When becoming a root, local transform IS the world transform
    glm::vec3 localRotation;
    Math::DecomposeTransform(oldWorldTransform, tc.LocalPosition, localRotation,
                             tc.LocalScale);
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

  // 1) Clear stale child lists first so no parent is wiped mid-build
  for (auto entity : view) {
    view.get<RelationshipComponent>(entity).Children.clear();
  }

  // 2) Gather and reverse to preserve forward creation order
  std::vector<entt::entity> orderedEntities(view.begin(), view.end());
  std::reverse(orderedEntities.begin(), orderedEntities.end());

  // 3) Re-link children to parents
  for (auto entity : orderedEntities) {
    auto [id, relation] = view.get<IDComponent, RelationshipComponent>(entity);
    if (relation.Parent != 0) {
      if (Entity parent = GetEntityByUUID(relation.Parent)) {
        parent.GetComponent<RelationshipComponent>().Children.push_back(id.ID);
      }
    }
  }
}

void Scene::SyncEntityComponentsFromPrefab(Entity dstEntity, Entity srcPrefabEntity, bool preserveInstanceTransform) {
  TransformComponent savedTransform;
  bool savedSelfActive = true;
  if (dstEntity.HasComponent<TransformComponent>()) {
    savedTransform = dstEntity.GetComponent<TransformComponent>();
  }
  if (dstEntity.HasComponent<ObjectStateComponent>()) {
    savedSelfActive = dstEntity.GetComponent<ObjectStateComponent>().SelfActive;
  }

  // Copy/overwrite components present on the prefab entity (preserving IDComponent and RelationshipComponent)
  CopyComponentIfExists<
      TagComponent,
      LayerComponent,
      ObjectStateComponent,
      TransformComponent,
      CameraComponent,
      SpriteRendererComponent,
      CircleRendererComponent,
      Rigidbody2DComponent,
      BoxCollider2DComponent,
      CircleCollider2DComponent,
      ScriptComponentContainer
  >(dstEntity, srcPrefabEntity);

  if (preserveInstanceTransform && dstEntity.HasComponent<TransformComponent>()) {
    dstEntity.GetComponent<TransformComponent>() = savedTransform;
  }

  if (dstEntity.HasComponent<ObjectStateComponent>()) {
    dstEntity.GetComponent<ObjectStateComponent>().SelfActive = savedSelfActive;
  }

  if (dstEntity.HasComponent<ScriptComponentContainer>()) {
    ScriptEngine::GetMutable().CopyScriptFields(srcPrefabEntity, dstEntity);
  }

  // Remove components that exist on dstEntity but were removed from srcPrefabEntity
  auto removeIfMissingOnPrefab = [&]<typename T>() {
    if constexpr (!std::is_same_v<T, IDComponent> &&
                  !std::is_same_v<T, RelationshipComponent> &&
                  !std::is_same_v<T, PrefabLinkComponent>) {
      if (dstEntity.HasComponent<T>() && !srcPrefabEntity.HasComponent<T>()) {
        dstEntity.RemoveComponent<T>();
      }
    }
  };

  removeIfMissingOnPrefab.template operator()<TagComponent>();
  removeIfMissingOnPrefab.template operator()<LayerComponent>();
  removeIfMissingOnPrefab.template operator()<ObjectStateComponent>();
  removeIfMissingOnPrefab.template operator()<TransformComponent>();
  removeIfMissingOnPrefab.template operator()<CameraComponent>();
  removeIfMissingOnPrefab.template operator()<SpriteRendererComponent>();
  removeIfMissingOnPrefab.template operator()<CircleRendererComponent>();
  removeIfMissingOnPrefab.template operator()<Rigidbody2DComponent>();
  removeIfMissingOnPrefab.template operator()<BoxCollider2DComponent>();
  removeIfMissingOnPrefab.template operator()<CircleCollider2DComponent>();
  removeIfMissingOnPrefab.template operator()<ScriptComponentContainer>();
}

void Scene::SyncPrefabHierarchyRecursive(Entity dstEntity, Entity srcPrefabEntity, AssetHandle prefabID, uint64_t prefabVersion) {
  if (!dstEntity || !srcPrefabEntity)
    return;

  SyncEntityComponentsFromPrefab(dstEntity, srcPrefabEntity, /*preserveInstanceTransform=*/true);

  if (dstEntity.HasComponent<PrefabLinkComponent>()) {
    auto &link = dstEntity.GetComponent<PrefabLinkComponent>();
    link.PrefabID = prefabID;
    link.LastKnownUpdate = prefabVersion;
  }

  auto &dstRel = dstEntity.GetComponent<RelationshipComponent>();
  auto &srcRel = srcPrefabEntity.GetComponent<RelationshipComponent>();

  // Match/create child entities in prefab child order while preserving existing child UUIDs/order where possible
  std::vector<UUID> updatedChildren;
  updatedChildren.reserve(srcRel.Children.size());

  std::vector<UUID> unmatchedExistingChildren = dstRel.Children;

  for (UUID srcChildUUID : srcRel.Children) {
    Entity srcChild = srcPrefabEntity.GetScene()->GetEntityByUUID(srcChildUUID);
    if (!srcChild)
      continue;

    const std::string &srcChildName = srcChild.GetComponent<TagComponent>().Tag;

    // Try matching an existing child by name first
    UUID matchedUUID = 0;
    for (auto it = unmatchedExistingChildren.begin(); it != unmatchedExistingChildren.end(); ++it) {
      Entity candidate = GetEntityByUUID(*it);
      if (candidate && candidate.GetComponent<TagComponent>().Tag == srcChildName) {
        matchedUUID = *it;
        unmatchedExistingChildren.erase(it);
        break;
      }
    }

    Entity dstChild;
    if (matchedUUID != 0) {
      dstChild = GetEntityByUUID(matchedUUID);
    } else {
      dstChild = CreateEntity(srcChildName, dstEntity.GetUUID());
    }

    if (dstChild) {
      dstChild.GetComponent<RelationshipComponent>().Parent = dstEntity.GetUUID();
      updatedChildren.push_back(dstChild.GetUUID());
      SyncPrefabHierarchyRecursive(dstChild, srcChild, prefabID, prefabVersion);
    }
  }

  // Delete child entities that were removed from the prefab
  for (UUID leftoverChildUUID : unmatchedExistingChildren) {
    Entity leftoverChild = GetEntityByUUID(leftoverChildUUID);
    if (leftoverChild) {
      DeleteEntity(leftoverChild);
    }
  }

  dstRel.Children = std::move(updatedChildren);
}

void Scene::RecursiveUpdatePrefabs(const UUID &entityID) {
  Entity entity = GetEntityByUUID(entityID);
  if (!entity)
    return;

  // Snapshot children before potential structural changes
  std::vector<UUID> childSnapshot = entity.GetComponent<RelationshipComponent>().Children;

  bool updatedThisNode = false;
  if (entity.HasComponent<PrefabLinkComponent>()) {
    auto &link = entity.GetComponent<PrefabLinkComponent>();
    Asset::MetaData metaData = Project::GetActive()->GetEditorAssetManager()->GetMetaData(link.PrefabID);

    if (metaData.LastUpdated > link.LastKnownUpdate) {
      // Load/fetch the prefab's template scene & root entity for link.PrefabID
      Ref<Prefab> prefab = AssetManager::GetAsset<Prefab>(link.PrefabID); // adapt to your PrefabManager API
      Ref<Scene> prefabScene = prefab->GetScene();
      if (prefabScene) {
        UUID rootEntityID = prefabScene->m_RootEntities[0];
        Entity prefabRoot = prefabScene->GetEntityByUUID(rootEntityID); // adapt to your prefab root lookup
        if (prefabRoot) {
          SyncPrefabHierarchyRecursive(entity, prefabRoot, link.PrefabID, metaData.LastUpdated);
          updatedThisNode = true;
        }
      }
    }
  }

  // If this node wasn't just synced from its own prefab root, recurse into existing children
  // (or still recurse if nested prefabs can have independent PrefabLinkComponents)
  if (!updatedThisNode) {
    for (const UUID &childID : childSnapshot) {
      RecursiveUpdatePrefabs(childID);
    }
  }
}

void Scene::UpdateAllPrefabs() {
  for (auto &entity : m_RootEntities) {
    RecursiveUpdatePrefabs(entity);
  }
}

void Scene::ExecuteSceneCommandQueue() {
  std::vector<std::function<void()>> copy;
  {
    std::scoped_lock<std::mutex> lock(m_SceneCommandQueueMutex);
    copy = m_SceneCommandQueue;
    m_SceneCommandQueue.clear();
  }

  for (auto &function : copy)
    function();

}


// On Component Added

template <typename T>
void Scene::OnComponentAdded(Entity entity, T &component) {
  static_assert(false, "Component Not Supported!");
}
template <>
void Scene::OnComponentAdded<LayerComponent>(Entity entity, LayerComponent &component) {}

template <>
void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent &component) {}

template <>
void Scene::OnComponentAdded<ObjectStateComponent>(Entity entity, ObjectStateComponent &component) {}

template <>
void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent &component) {
  if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
    component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
}

template <>
void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent &component) {}

template <>
void Scene::OnComponentAdded<CircleRendererComponent>(Entity entity, CircleRendererComponent &component) {}

template <>
void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent &component) {}

template <>
void Scene::OnComponentAdded<Rigidbody2DComponent>(Entity entity, Rigidbody2DComponent &component) {
  if (!m_IsRunning)
    return;

  Physics2D::Get().EnableRigidbody(entity, true);
}

template <>
void Scene::OnComponentAdded<NameComponent>(Entity entity, NameComponent &component) {}

template <>
void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent &component) {
  if (!m_IsRunning)
    return;

  Physics2D::Get().EnableCollider(entity, true);
}

template <>
void Scene::OnComponentAdded<CircleCollider2DComponent>(Entity entity, CircleCollider2DComponent &component) {
  if (!m_IsRunning)
    return;

  Physics2D::Get().EnableCollider(entity, true);
}

template <>
void Scene::OnComponentAdded<ScriptComponentContainer>(Entity entity, ScriptComponentContainer &component) {
  for (auto &sc : component.Scripts)
    sc.Loaded = false;

  if (!m_IsRunning)
    return;

  ScriptEngine::GetMutable().OnStartEntity(entity);
}

template <>
void Scene::OnComponentAdded<PrefabLinkComponent>(Entity entity, PrefabLinkComponent &component) {}

template <>
void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent &component) {}

template <>
void Scene::OnComponentAdded<RelationshipComponent>(Entity entity, RelationshipComponent &component) {}

} // namespace CatEngine
