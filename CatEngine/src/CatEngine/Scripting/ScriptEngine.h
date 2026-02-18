#include <filesystem>

#include "CatEngine/Core/UUID.h"
#include "ScriptEntityStorage.h"

namespace CatEngine {
class Scene;
class Entity;

enum class CollisionType {
  None = 0,
  Begin,
  End,
};

////////////////////////////////////////////////////////////////////////////
// Script Engine ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

class ScriptClass;
class ScriptInstance;

class ScriptEngine {
public:
  void Init();
  void Shutdown();

  void InitializeFileSystems();

  void SetSceneContext(Ref<Scene> scene);
  void OnRuntimeStop();

  void ReloadBinaries();

  bool ScriptClassExists(const char *className);
  void OnStartEntity(Entity e);
  void OnUpdateEntity(Entity e, float ts);

  void DispatchCollisionEvent(UUID entityA, UUID entityB, CollisionType type);

  Ref<Scene> GetSceneContext();
  Ref<ScriptInstance> GetEntityScriptInstance(UUID entityID);

  CapyObject *GetManagedInstance(UUID uuid);

  const ScriptFieldMap &GetInitializedFields(const UUID &entityID) const;

  std::unordered_map<uint64_t, Ref<ScriptClass>> &GetScriptClasses();
  Ref<ScriptClass> GetScriptClass(const char *className);
  ScriptFieldMap &GetScriptFieldMap(Entity entity);
  ScriptFieldMap &GetScriptFieldMap(UUID entity);

  UUID GetUUIDFromStringHash(const std::string &nameSpace,
                             const std::string &className,
                             const std::string &fieldName);

  static ScriptEngine &GetMutable();

private:
  void InitCapy();
  void ShutdownCapy();

  CapyObject *InstantiateClass(CapyClass *capyClass);
  void LoadBinaryClasses();

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
  ScriptClass(const char *nameSpace, const char *className);

  ~ScriptClass() { m_ClassObject.release(); }

  CapyObject *Instantiate(UUID entityID);
  CapyMethod *GetMethod(const char *methodName);
  void *InvokeMethod(CapyObject *instance, CapyMethod *method,
                     const std::vector<RuntimeValue> &values);

  const char *GetClassName() const { return m_ClassName; }

  const std::unordered_map<UUID, ScriptField> &GetFields() const {
    return m_Fields;
  }

private:
  const char *m_ClassNamespace;
  const char *m_ClassName;

  std::unordered_map<UUID, ScriptField> m_Fields;

  std::unique_ptr<CapyObject> m_ClassObject;
  CapyClass *m_CapyClass = nullptr;

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

  template <typename T> void SetFieldData(const std::string &name, T value) {
    static_assert(sizeof(T) <= 128, "Type to large!");
    SetFieldDataInternal(name, &value);
  }

  CapyObject *GetManagedObject() { return m_Instance; }

private:
  bool GetFieldDataInternal(const std::string &name, void *buffer);
  void SetFieldDataInternal(const std::string &name, void *value);

private:
  Ref<ScriptClass> m_ScriptClass;

  CapyObject *m_Instance = nullptr;
  CapyMethod *m_StartMethod = nullptr;
  CapyMethod *m_UpdateMethod = nullptr;
  CapyMethod *m_CollisionEnterMethod = nullptr;
  CapyMethod *m_CollisionExitMethod = nullptr;

  std::unordered_map<std::string, std::vector<uint8_t>> m_DefaultFieldDatas;

  static inline char s_FieldValueBuffer[8];

  friend class ScriptEngine;
  friend struct ScriptFieldInstance;
};

} // namespace CatEngine
