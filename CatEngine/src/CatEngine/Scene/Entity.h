#pragma once

#include "CatEngine/Core/Hash.h"
#include "CatEngine/Scene/Components/Scripting/ScriptComponentContainer.h"
#include "Components/Components.h"
#include "Scene.h"

#include <cstdint>
#include <entt.hpp>

namespace CatEngine {
class Entity {
public:
  Entity() = default;
  Entity(entt::entity handle, Scene *scene);
  Entity(const Entity &other) = default;

  entt::entity &GetEntityID() { return m_EntityHandle; }

  template <typename T, typename... Args> T &AddComponent(Args &&...args) {
    CE_API_ASSERT(!HasComponent<T>(), "component already exists!");
    T &component = m_Scene->m_Registry.emplace<T>(m_EntityHandle,
                                                  std::forward<Args>(args)...);
    m_Scene->OnComponentAdded<T>(*this, component);
    return component;
  }

  ScriptComponent &AddScriptComponent() {
    if (!HasComponent<ScriptComponentContainer>()) {
      AddComponent<ScriptComponentContainer>();
    }

    auto& container = GetComponent<ScriptComponentContainer>();

    // Emplace the script component with its class name
    ScriptComponent sc;
    container.Scripts.push_back(sc);

    return container.Scripts.at(container.Scripts.size() - 1);
  }

  template <typename T, typename... Args>
  T &AddOrReplaceComponent(Args &&...args) {
    T &component = m_Scene->m_Registry.emplace_or_replace<T>(
        m_EntityHandle, std::forward<Args>(args)...);
    m_Scene->OnComponentAdded<T>(*this, component);
    return component;
  }

  template <typename T> T &GetComponent() {
    CE_API_ASSERT(HasComponent<T>(), "Entity does not have component!");

    return m_Scene->m_Registry.get<T>(m_EntityHandle);
  }

  template <typename T> T &GetOrAddComponent() {
    if (HasComponent<T>()) {
      return GetComponent<T>();
    }
    return AddComponent<T>();
  }

  template <typename T> void RemoveComponent() {
    CE_API_ASSERT(HasComponent<T>(), "Entity does not have component!");

    m_Scene->m_Registry.remove<T>(m_EntityHandle);
  }

  void RemoveScriptComponent(const int &index) {
    CE_API_ASSERT(HasComponent<ScriptComponentContainer>(), "Script Component Container doesn't exist!");
    auto &scc = GetComponent<ScriptComponentContainer>();
    int idx = 0;
    for (auto it = scc.Scripts.begin(); it != scc.Scripts.end(); ++it) {
      if (idx++ == index) {
        scc.Scripts.erase(it);
        break;
      }
    }
  }

  template <typename T> bool HasComponent() {
    return m_Scene->m_Registry.try_get<T>(m_EntityHandle);
  }

  template <typename T> T &ResetComponent() {
    T &component = m_Scene->m_Registry.emplace<T>(m_EntityHandle);
    return component;
  }

  template <typename T, typename... Args>
  T &AddComponent(Args &&...args) const {
    CE_API_ASSERT(!HasComponent<T>(), "Entity already has component!");
    T &component = m_Scene->m_Registry.emplace<T>(m_EntityHandle,
                                                  std::forward<Args>(args)...);
    m_Scene->OnComponentAdded<T>(*this, component);
    return component;
  }

  template <typename T, typename... Args>
  T &AddOrReplaceComponent(Args &&...args) const {
    T &component = m_Scene->m_Registry.emplace_or_replace<T>(
        m_EntityHandle, std::forward<Args>(args)...);
    m_Scene->OnComponentAdded<T>(*this, component);
    return component;
  }

  template <typename T> T &GetComponent() const {
    CE_API_ASSERT(HasComponent<T>(), "Entity does not have component!");

    return m_Scene->m_Registry.get<T>(m_EntityHandle);
  }

  ScriptComponent &GetScriptComponent(const UUID &scriptHash) {
    CE_API_ASSERT(!HasComponent<ScriptComponentContainer>(), "Script Component Container doesn't exist!");
    auto &scc = GetComponent<ScriptComponentContainer>();
    for (auto it = scc.Scripts.begin(); it != scc.Scripts.end(); ++it) {
      auto &script = *it;
      if (scriptHash == script.ScriptID) {
        return script;
      }
    }
    CE_API_ASSERT(false, "ScriptComponentContainer has no script with hash!");
  }

  template <typename T> T &GetOrAddComponent() const {
    if (HasComponent<T>()) {
      return GetComponent<T>();
    }
    return AddComponent<T>();
  }

  template <typename T> void RemoveComponent() const {
    CE_API_ASSERT(HasComponent<T>(), "Entity does not have component!");

    m_Scene->m_Registry.remove<T>(m_EntityHandle);
  }

  template <typename T> bool HasComponent() const {
    return m_Scene->m_Registry.try_get<T>(m_EntityHandle);
  }

  template <typename T> T &ResetComponent() const {
    T &component = m_Scene->m_Registry.emplace<T>(m_EntityHandle);
    return component;
  }

  glm::mat4 GetWorldTransform() const {
    if (!HasComponent<TransformComponent>() || !HasComponent<RelationshipComponent>()) {
        return glm::mat4(1.0f);
    }

    const auto &tc = GetComponent<TransformComponent>();
    const auto &rc = GetComponent<RelationshipComponent>();

    // Base case: Root entity (no parent), world transform is its local transform
    if (rc.Parent == 0) {
        return tc.GetLocalTransform();
    }

    // Find the parent entity in the scene using its UUID
    Entity parentEntity = m_Scene->GetEntityByUUID(rc.Parent);
    if (!parentEntity) {
        // Fallback if the parent reference is broken or invalid
        return tc.GetLocalTransform();
    }

    // Recursive step: Parent's World Transform * This Entity's Local Transform
    return parentEntity.GetWorldTransform() * tc.GetLocalTransform();
  }

  bool IsChild(const UUID &entityID) {
    if (!HasComponent<RelationshipComponent>())
      return false;

    auto& rc = GetComponent<RelationshipComponent>();

    // Check direct children
    for (UUID childUUID : rc.Children) {
      if (childUUID == entityID)
        return true;

      // Recursively check deeper descendants
      Entity childEntity = m_Scene->GetEntityByUUID(childUUID);
      if (childEntity && childEntity.IsChild(entityID)) {
        return true;
      }
    }

    return false;
  }

  void SetSceneContext(Ref<Scene> scene) {
    m_Scene = scene.get();
  }

  operator bool() const { return m_EntityHandle != entt::null; }
  operator uint32_t() const { return (uint32_t)m_EntityHandle; }
  operator entt::entity() const { return m_EntityHandle; }

  UUID GetUUID() { return GetComponent<IDComponent>().ID; }
  const std::string &GetName() {
    static std::string nullScene = "<null scene>";
    if (!m_Scene)
      return nullScene;
    return GetComponent<NameComponent>().Name;
  }

  bool operator==(const Entity &other) const {
    return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
  }
  bool operator!=(const Entity &other) const { return !(*this == other); }

private:
  entt::entity m_EntityHandle{entt::null};
  Scene *m_Scene = nullptr;
};
} // namespace CatEngine
