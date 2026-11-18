#pragma once

#include <entt.hpp>
#include <vector>

#include "CatEngine/Core/TimeStep.h"
#include "CatEngine/Core/UUID.h"
#include "CatEngine/Renderer/EditorCamera.h"

#include "CatEngine/AssetManager/Asset.h"

class b2World;

namespace CatEngine {

class Entity;

class Scene : public Asset {
public:
  Scene();
  ~Scene();

  void CopyTo(Ref<Scene> &target);

  Entity CreateEntity(const std::string &name = std::string());
  Entity CreateEntityWithUUID(UUID uuid, const std::string &name = std::string(), UUID parent = 0);
  void DeleteEntity(Entity entity);

  void SetEntityActive(Entity entity, bool active);
  void SetEntityChildActiveRecursive(Entity child, bool parentActive);

  Entity InstantiatePrefab(const UUID &prefabID, const UUID &parentID = 0);

  void DrawEntityRecursive(Entity e, const glm::mat4 &parentWorldMatrix);

  void OnPauseStart();
  void OnPauseStop();

  void OnRuntimeStart();
  void OnRuntimeStop();

  void OnSimulationStart();
  void OnSimulationStop();

  void OnUpdateEditor(Time ts, EditorCamera &camera);
  void OnUpdateMainCameraPreview(
      Time ts); // This is outside of the runtime, used for the camera preview
  void OnUpdateSimulation(Time time, EditorCamera &camera);
  void OnUpdateRuntime(Time ts);

  void ShutdownRuntime();

  void OnViewportResize(uint32_t width, uint32_t height);

  void DuplicateEntity(Entity entity);
  Entity PasteEntity(Entity entity);

  Entity FindEntityByName(const std::string_view &name);
  Entity GetEntityByUUID(UUID entityID);

  Entity GetPrimaryCameraEntity();

  template <typename... Components> auto GetAllComponentsWith() {
    return m_Registry.view<Components...>();
  }

  entt::registry &GetReg() { return m_Registry; }

  bool IsRunning() { return m_IsRunning; }

  void ReparentEntity(Entity e, Entity newParent);
  void MoveEntityBefore(Entity e, Entity targetEntity);
  void MoveEntityAfter(Entity e, Entity targetEntity);

  void RegisterParents();

  std::vector<UUID> &GetRootEntities() { return m_RootEntities; }

  static AssetType GetStaticType() { return AssetType::Scene; }
  virtual AssetType GetType() const override { return GetStaticType(); }

private:
  template <typename T> void OnComponentAdded(Entity entity, T &component);


private:
  void OnPhysics2DStart();
  void OnPhysics2DUpdate(Time ts);
  void OnPhysics2DStop();

  void OnScriptStart();

  void UpdateTransforms();

  void CalculateTransformRecursive(Entity e, const glm::mat4 &parentWorldMatrix);

  void DeleteEntities();
private:
  void ExecuteSceneCommandQueue();
private:
  entt::registry m_Registry;
  uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
  float m_FixedTimeStep = 0.02f;
  float m_Accumulator = 0.0f;

  std::vector<UUID> m_DeletedEntities;
  std::unordered_map<UUID, entt::entity> m_EntityMap;
  std::vector<UUID> m_RootEntities;

  bool m_IsRunning = false;
  bool m_IsPaused = false;

  std::vector<std::function<void()>> m_SceneCommandQueue;
  std::mutex m_SceneCommandQueueMutex;

  friend class Entity;
  friend class Physics2D;
};
} // namespace CatEngine
