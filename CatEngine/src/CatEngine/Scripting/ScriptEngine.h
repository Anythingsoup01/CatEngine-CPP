#pragma once
#include "cepch.h"

#include "CatEngine/Core/UUID.h"
#include "ScriptEntityStorage.h"

#include "CatEngine/Scene/Entity.h"
#include "Capybara/Core/ReflectionTypes.h"
#include <cstdint>
#include <unordered_map>

namespace CatEngine {

enum class CollisionType {
  None = 0,
  Begin,
  End,
};

class ScriptClass;
class ScriptInstance;

class ScriptEngine {
public:
  ScriptEngine();

  void Init();
  void Shutdown();

  void SetSceneContext(Ref<Scene> scene);
  void OnRuntimeStop();

  void PollJIT();

  bool ScriptClassExists(const std::string_view &className);

  void OnStartEntity(Entity e);
  void OnUpdateEntity(Entity e, float ts);

  void DispatchCollisionEvent(UUID entityA, UUID entityB, CollisionType type);

  Ref<Scene> GetSceneContext();
  Ref<ScriptInstance> GetEntityScriptInstance(UUID entityID);

  Capybara::CapyObject::Instance *GetManagedInstance(UUID uuid);

  const ScriptFieldMap &GetInitializedFields(const UUID &entityID) const;

  std::unordered_map<uint64_t, Ref<ScriptClass>> &GetScriptClasses();
  Ref<ScriptClass> GetScriptClass(const char *className);
  ScriptFieldMap &GetScriptFieldMap(Entity entity);
  ScriptFieldMap &GetScriptFieldMap(UUID entity);

  static ScriptEngine &GetMutable() { return *s_Instance; }

private:
  void CapyInit();
  void CapyShutdown();

  Ref<Capybara::CapyObject> InstantiateClass(const std::string_view &className);
  void LoadBinaryClasses();
private:
  static inline ScriptEngine *s_Instance;
private:
  friend class ScriptGlue;
  friend class Application;
  friend class Scene;
  friend class SceneSerializer;
  friend class SceneHierarchyPanel;
  friend class ScriptInstance;
};

////////////////////////////////////////////////////////////////////////////
// Script Class ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

class ScriptClass {
public:
  ScriptClass() = default;
  ScriptClass(const std::string &className);

  Ref<Capybara::CapyObject::Instance> Instantiate(UUID entityID);

  template <typename Ret, typename... Args>
  Ret InvokeMethod(const std::string &methodName, Ref<Capybara::CapyObject::Instance> instance, Args &&...args) {
    return instance->Invoke<Ret>(methodName, m_ClassObject.get(), std::forward<Args>(args)...);
  }

  const char *GetClassName() const { return m_ClassName.c_str(); }

  const std::unordered_map<UUID, ScriptField> &GetFields() const {
    return m_Fields;
  }

private:
  const char *m_ClassNamespace;
  const std::string m_ClassName;

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

  void InvokeUpdateMethod(float ts);
  void InvokeStartMethod();

  void InvokeOnCollisionEnter(const UUID &other);
  void InvokeOnCollisionExit(const UUID &other);

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

  Ref<Capybara::CapyObject::Instance> GetManagedObject() { return m_Instance; }

private:
  template<typename T>
  bool GetFieldDataInternal(const std::string &name, T *buffer) {
    m_Instance->GetField(name, m_ScriptClass->m_ClassObject.get(), buffer);
    if (!buffer)
      return false;

    return true;
  }

  template<typename T>
  bool SetFieldDataInternal(const std::string &name, T &value) {
    if (m_Instance->SetField(name, value, m_ScriptClass->m_ClassObject.get())) {
      return true;
    }

    return false;
  }

  bool GetFieldDataInternal(const std::string &name, void *buffer, const uint64_t &size) {
    m_Instance->GetFieldVoid(name, m_ScriptClass->m_ClassObject.get(), buffer, size);
    if (!buffer)
      return false;

    return true;
  }

  bool SetFieldDataInternal(const std::string &name, void *value, const uint64_t &size) {
    if (m_Instance->SetFieldVoid(name, m_ScriptClass->m_ClassObject.get(), value, size)) {
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

}
