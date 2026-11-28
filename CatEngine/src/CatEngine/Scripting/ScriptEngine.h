#pragma once

#include "ScriptEntityStorage.h"

#include "CatEngine/Core/Buffer.h"
#include "CatEngine/Core/UUID.h"
#include "CatEngine/Scene/Entity.h"

#include <Capybara/Core/ReflectionTypes.h>

namespace CatEngine {

enum class CollisionType {
  None = 0,
  Begin,
  End,
};


////////////////////////////////////////////////////////////////////////////
// Script Class ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

class ScriptClass {
public:
  ScriptClass() = default;
  ScriptClass(const std::string &className);

  Ref<Capybara::CapyObject::Instance> Instantiate(UUID entityID);
  void Destroy(Ref<Capybara::CapyObject::Instance> instance);

  template <typename Ret, typename... Args>
  Ret InvokeMethod(const std::string &methodName,
                   Ref<Capybara::CapyObject::Instance> instance,
                   Args &&...args) {
    return instance->Invoke<Ret>(methodName, m_ClassObject.get(), std::forward<Args>(args)...);
  }

  const char *GetClassName() const { return m_ClassName.c_str(); }

  const std::unordered_map<UUID, ScriptField> &GetFields() const {
    return m_Fields;
  }

private:
  const std::string m_ClassName;

  std::unordered_set<std::string> m_Methods;
  uint32_t m_UIHandlerBits = 0;
  uint32_t m_UIMask = 0;

  std::unordered_map<UUID, ScriptField> m_Fields;
  Ref<Capybara::CapyObject> m_ClassObject;

  friend class ScriptEngine;
  friend class ScriptInstance;
};

////////////////////////////////////////////////////////////////////////////
// Script Instance /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

class ScriptInstance {
public:
  ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity);
  ~ScriptInstance();

  void DestroyInstance();

  bool HasMethod(const std::string &name) { return m_ScriptClass->m_Methods.contains(name); }
  bool InvokeUIEvent(const UIEvent &ev);       // returns "consumed"

  template <typename Ret = void, typename... Args>
  Ret Invoke(const std::string &name, Args &&...args) {
    if constexpr (std::is_void_v<Ret>) {
      if (HasMethod(name))
        m_ScriptClass->InvokeMethod<void>(name.c_str(), m_Instance, std::forward<Args>(args)...);
    } else {
      if (!HasMethod(name)) return Ret{};
      return m_ScriptClass->InvokeMethod<Ret>(name.c_str(), m_Instance, std::forward<Args>(args)...);
    }
  }

  Ref<ScriptClass> GetScriptClass() { return m_ScriptClass; }

  template <typename T> T GetFieldData(const std::string &name) {
    static_assert(sizeof(T) <= 128, "Type to large!");
    bool success = GetFieldDataInternal(name, &s_FieldValueBuffer);
    if (success)
      return *(T *)s_FieldValueBuffer;

    return T();
  }

  template <typename T> void SetFieldData(const std::string &name, T &value) {
    static_assert(sizeof(T) <= 128, "Type to large!");
    SetFieldDataInternal(name, value);
  }

  VectorBuffer GetVectorFieldData(const std::string &name) {
    VectorBuffer out;
    GetVectorFieldDataInternal(name, &out);
    return out;
  }

  Ref<Capybara::CapyObject::Instance> GetManagedObject() { return m_Instance; }

private:
  template <typename T>
  bool GetFieldDataInternal(const std::string &name, T *buffer) {
    m_Instance->GetField(name, m_ScriptClass->m_ClassObject.get(), buffer);
    if (!buffer)
      return false;

    return true;
  }

  template <typename T>
  bool SetFieldDataInternal(const std::string &name, T &value) {
    if (m_Instance->SetField(name, value, m_ScriptClass->m_ClassObject.get())) {
      return true;
    }

    return false;
  }

  bool GetFieldDataInternal(const std::string &name, void *buffer,
                            const uint64_t &size) {
    m_Instance->GetFieldVoid(name, m_ScriptClass->m_ClassObject.get(), buffer,
                             size);
    if (!buffer)
      return false;

    return true;
  }

  bool SetFieldDataInternal(const std::string &name, void *value,
                            const uint64_t &size) {
    if (m_Instance->SetFieldVoid(name, m_ScriptClass->m_ClassObject.get(),
                                 value, size)) {
      return true;
    }

    return false;
  }

  bool GetVectorFieldDataInternal(const std::string &name, VectorBuffer *out) {
    std::vector<uint8_t> tmp;
    if (m_Instance->GetFieldVector(name, m_ScriptClass->m_ClassObject.get(),
                                   &tmp)) {
      *out = VectorBuffer(tmp);
      return true;
    }

    return false;
  }

private:
  Ref<ScriptClass> m_ScriptClass;

  Ref<Capybara::CapyObject::Instance> m_Instance = nullptr;

  static inline char s_FieldValueBuffer[8];

  friend class ScriptEngine;
  friend struct ScriptFieldInstance;
};


class ScriptEngine {
public:
  ScriptEngine();

  void Init();
  void Shutdown();

  void SetSceneContext(Ref<Scene> scene);
  void OnRuntimeStop();

  void PollJIT();

  bool ScriptClassExists(const std::string_view &className);

  std::vector<Ref<ScriptInstance>> GetActiveScriptInstances(Entity entity, bool requireLoaded = false);
  Ref<ScriptInstance> FindScript(Entity entity, std::string_view className);

  uint32_t GetUIHandlerMask(UUID entityID);            // union over the entity's live scripts
  bool DispatchUIEvent(Entity target, const UIEvent &ev);

  // Call a public method on one named script. Returns whether it was called.
  template <typename... Args>
  bool Invoke(Entity e, std::string_view cls, const std::string &method, Args &&...args) {
    auto s = FindScript(e, cls);
    if (!s || !s->HasMethod(method)) return false;
    s->Invoke<void>(method, std::forward<Args>(args)...);
    return true;
  }

  template <typename Ret, typename... Args>
  std::optional<Ret> InvokeReturn(Entity e, std::string_view cls, const std::string &method, Args &&...args) {
    auto s = FindScript(e, cls);
    if (!s || !s->HasMethod(method)) return std::nullopt;
    return s->Invoke<Ret>(method, std::forward<Args>(args)...);
  }

  // SendMessage-style: calls the method on every live script of the entity that has it.
  template <typename... Args>
  int Broadcast(Entity e, const std::string &method, Args &&...args) {
    int n = 0;
    for (auto &s : GetActiveScriptInstances(e))
    if (s->HasMethod(method)) { s->Invoke<void>(method, args...); ++n; }
    return n;
  }

  void LoadEntityScriptInstances(Entity entity);
  void LoadEnityFieldInstances(Entity entity);
  void OnStartEntity(Entity entity);

  void LoadRuntimeScripts();

  void OnUpdate(Time time);
  void OnFixedUpdate(Time time);

  void DeleteEntity(Entity entity);

  void DispatchCollisionEvent(UUID entityA, UUID entityB, CollisionType type);

  Ref<Scene> GetSceneContext();
  Ref<ScriptInstance> GetEntityScriptInstance(const UUID &entityID,
                                              const UUID &scriptID);

  Capybara::CapyObject::Instance *GetManagedInstance(UUID uuid);

  const FieldMap &GetInitializedFields(const UUID &entityID,
                                       const UUID &scriptID) const;

  std::unordered_map<UUID, Ref<ScriptClass>> &GetScriptClasses();
  Ref<ScriptClass> GetScriptClass(const char *className);
  ScriptFieldMap &GetScriptFieldMap(Entity entity);
  ScriptFieldMap &GetScriptFieldMap(UUID entity);

  void CopyScriptFields(Entity src, Entity dst);

  static ScriptEngine &GetMutable() { return *s_Instance; }

private:
  void CapyInit();
  void CapyShutdown();

  Ref<Capybara::CapyObject> InstantiateClass(const std::string_view &className);
  void LoadBinaryClasses();

  void OnUpdateEntity(Entity e, float ts);
  void OnFixedUpdateEntity(Entity e, float ts);

  void ExecuteCommandQueue();

private:
  static inline ScriptEngine *s_Instance;

  std::vector<std::function<void()>> m_CommandQueue;
  std::mutex m_CommandQueueMutex;

private:
  friend class ScriptGlue;
  friend class Application;
  friend class Scene;
  friend class SceneSerializer;
  friend class SceneHierarchyPanel;
  friend class ScriptInstance;
};

} // namespace CatEngine
