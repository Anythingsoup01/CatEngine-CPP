#include "ScriptEngine.h"
#include "cepch.h"

#include <capybara/capybara.h>

#include "CatEngine/Scene/Entity.h"
#include "CatEngine/Scene/Scene.h"

#include <glm/glm.hpp>

#include "CatEngine/Core/Application.h"
#include "CatEngine/Core/Hash.h"

#include "CatEngine/Scene/Components/Components.h"

#include "CatEngine/Project/Project.h"
#include "CatEngine/Scripting/ScriptGlue.h"

namespace CatEngine {
static std::unordered_map<std::string, DataType> s_DataTypeLookup = {
    {"short", DataType::Short},
    {"int16_t", DataType::Short},
    {"unsigned short", DataType::UShort},
    {"uint16_t", DataType::UShort},
    {"int", DataType::Int},
    {"int32_t", DataType::Int},
    {"unsigned int", DataType::UInt},
    {"uint32_t", DataType::UInt},
    {"long", DataType::Long},
    {"int64_t", DataType::Long},
    {"unsigned long", DataType::ULong},
    {"uint64_t", DataType::ULong},

    {"float", DataType::Float},
    {"double", DataType::Double},

    {"bool", DataType::Bool},

    {"Vector2", DataType::Vector2},
    {"Vector3", DataType::Vector3},
    {"Vector4", DataType::Vector4},

    {"Object", DataType::Entity},

    {"Texture2D", DataType::Texture2D},
};

struct ScriptEngineData {
  CapyDomain *RootDomain;

  CapyImage *AppBinaryImage;

  std::filesystem::path CoreLibraryPath;

  std::unordered_map<uint64_t, Ref<ScriptClass>> EntityClasses;
  std::unordered_map<UUID, Ref<ScriptInstance>> EntityInstances;

  std::unordered_map<UUID, ScriptFieldMap> EntityScriptFields;
  std::unordered_map<std::string, UUID> StoredFieldIDs;

  std::vector<std::function<void()>> MainThreadQueue;

  bool SourceFileReloadPending = false;
  bool BinaryReloadPending = false;

  Ref<Scene> SceneContext = nullptr;
};

static ScriptEngineData *s_ScriptData = nullptr;

void CustomFileSystemEventCallback(FileEventType type,
                                   const std::filesystem::path &path) {

  if (type == FileEventType::Delete)
    return;
  // s_ScriptData->ReloadTimer = Timer();

  bool isRunning = s_ScriptData->SceneContext->IsRunning();

  if (isRunning) {
    if (!s_ScriptData->SourceFileReloadPending) {
      s_ScriptData->SourceFileReloadPending = true;
      s_ScriptData->MainThreadQueue.emplace_back(
          []() { ScriptEngine::GetMutable().ReloadBinaries(); });
    } else if (s_ScriptData->SourceFileReloadPending) {
      s_ScriptData->MainThreadQueue.clear();
      s_ScriptData->MainThreadQueue.emplace_back(
          []() { ScriptEngine::GetMutable().ReloadBinaries(); });
    }

  } else if (!isRunning) {
    using namespace std::chrono;
    // std::this_thread::sleep_for(500ms);
    if (!s_ScriptData->SourceFileReloadPending) {
      s_ScriptData->SourceFileReloadPending = true;
      Application::Get().SubmitToMainThread(
          [path]() { ScriptEngine::GetMutable().ReloadBinaries(); });
    }
  }
}

////////////////////////////////////////////////////////////////////////////
// Script Engine ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void ScriptEngine::Init() {
  s_ScriptData = new ScriptEngineData();

  InitCapy();

  LoadBinaryClasses();

  ScriptGlue::RegisterComponents();
  ScriptGlue::RegisterFunctions();
}

void ScriptEngine::Shutdown() {
  s_ScriptData->EntityScriptFields.clear();
  s_ScriptData->EntityClasses.clear();
  delete s_ScriptData;

  ShutdownCapy();
}

void ScriptEngine::InitializeFileSystems() {
  const std::filesystem::path &path = Project::GetAssetDirectory();
  capy_set_source_path(path, CustomFileSystemEventCallback, true);

  auto *lib = capy_domain_library_open("CapyBinary.so", false);

  s_ScriptData->AppBinaryImage = lib->Image.get();

  ReloadBinaries();
}

void ScriptEngine::SetSceneContext(Ref<Scene> scene) {
  s_ScriptData->SceneContext = scene;
}

void ScriptEngine::OnRuntimeStop() {
  s_ScriptData->EntityInstances.clear();

  s_ScriptData->SceneContext = nullptr;

  if (s_ScriptData->SourceFileReloadPending ||
      !s_ScriptData->MainThreadQueue.empty())
    Application::Get().SubmitToMainThread(s_ScriptData->MainThreadQueue.at(0));
}

void ScriptEngine::ReloadBinaries() {
#ifdef CE_RELEASE
  // CE_CLI_TRACE(s_ScriptData->ReloadTimer.ElapsedMillis());
#endif
  // CE_API_TRACE(s_ScriptData->ReloadTimer.ElapsedMillis());

  capy_poll();

  Application::Get().SubmitToMainThread([this]() {
    s_ScriptData->RootDomain = capy_get_root_domain();

    auto *lib = capy_domain_library_open("CapyBinary.so", false);

    s_ScriptData->AppBinaryImage = lib->Image.get();

    LoadBinaryClasses();

    ScriptGlue::RegisterFunctions();
    ScriptGlue::RegisterComponents();

    CE_API_WARN(capy_dump_domain());
  });

  s_ScriptData->SourceFileReloadPending = false;
}

bool ScriptEngine::ScriptClassExists(const char *className) {
  uint64_t hash = Hash::GenerateFNVHash(className);
  return s_ScriptData->EntityClasses.find(hash) !=
         s_ScriptData->EntityClasses.end();
}

void ScriptEngine::OnStartEntity(Entity e) {

  const auto &sc = e.GetComponent<ScriptComponent>();

  if (ScriptEngine::ScriptClassExists(sc.ClassName.c_str())) {
    UUID entityID = e.GetUUID();

    Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(
        s_ScriptData->EntityClasses[Hash::GenerateFNVHash(sc.ClassName)], e);

    if (!instance)
      return;

    s_ScriptData->EntityInstances[entityID] = instance;

    CE_API_INFO(e.GetName());

    // Copy field value if we set it
    if (s_ScriptData->EntityScriptFields.find(entityID) !=
        s_ScriptData->EntityScriptFields.end()) {
      const ScriptFieldMap &scriptFieldMap =
          s_ScriptData->EntityScriptFields.at(entityID);

      for (const auto &[uuid, fieldInstance] : scriptFieldMap) {
        CE_API_WARN("Field Instance Name: {}", fieldInstance.Field.Name);
        instance->SetFieldDataInternal(fieldInstance.Field.Name,
                                       (void *)fieldInstance.m_Data.Data);
      }
    }

    instance->InvokeStartMethod();
  }
}

void ScriptEngine::OnUpdateEntity(Entity e, float ts) {
  UUID entityUUID = e.GetUUID();

  if (s_ScriptData->EntityInstances.find(entityUUID) !=
      s_ScriptData->EntityInstances.end()) {
    Ref<ScriptInstance> instance = s_ScriptData->EntityInstances[entityUUID];
    instance->InvokeUpdateMethod(ts);
  } else {
    CE_API_ERROR("Could not find script instance {}", (uint64_t)entityUUID);
  }
}

void ScriptEngine::DispatchCollisionEvent(UUID uuidA, UUID uuidB,
                                          CollisionType type) {
Fields:
  Ref<ScriptInstance> scriptA, scriptB;
  if (s_ScriptData->EntityInstances.find(uuidA) !=
      s_ScriptData->EntityInstances.end())
    scriptA = s_ScriptData->EntityInstances[uuidA];

  if (s_ScriptData->EntityInstances.find(uuidB) !=
      s_ScriptData->EntityInstances.end())
    scriptB = s_ScriptData->EntityInstances[uuidB];

  switch (type) {
  case CollisionType::Begin:
    if (scriptA)
      scriptA->InvokeOnCollisionEnter(uuidB);
    if (scriptB)
      scriptB->InvokeOnCollisionEnter(uuidA);
    break;
  case CollisionType::End:
    if (scriptA)
      scriptA->InvokeOnCollisionExit(uuidB);
    if (scriptB)
      scriptB->InvokeOnCollisionExit(uuidA);
    break;
  default:
    break;
  }
}

Ref<Scene> ScriptEngine::GetSceneContext() {
  return s_ScriptData->SceneContext;
}

Ref<ScriptInstance> ScriptEngine::GetEntityScriptInstance(UUID entityID) {
  auto it = s_ScriptData->EntityInstances.find(entityID);
  if (it == s_ScriptData->EntityInstances.end())
    return nullptr;

  return it->second;
}

CapyObject *ScriptEngine::GetManagedInstance(UUID uuid) {
  if (s_ScriptData->EntityInstances.find(uuid) !=
      s_ScriptData->EntityInstances.end())
    return s_ScriptData->EntityInstances.at(uuid)->GetManagedObject();

  return nullptr;
}

const ScriptFieldMap &
ScriptEngine::GetInitializedFields(const UUID &entityID) const {
  static ScriptFieldMap s_EmptyFieldMap = {};
  if (!s_ScriptData->EntityScriptFields.contains(entityID))
    return s_EmptyFieldMap;

  return s_ScriptData->EntityScriptFields.at(entityID);
}

std::unordered_map<uint64_t, Ref<ScriptClass>> &
ScriptEngine::GetScriptClasses() {
  return s_ScriptData->EntityClasses;
}

Ref<ScriptClass> ScriptEngine::GetScriptClass(const char *className) {
  uint64_t hash = Hash::GenerateFNVHash(className);
  if (s_ScriptData->EntityClasses.find(hash) !=
      s_ScriptData->EntityClasses.end())
    return s_ScriptData->EntityClasses.at(hash);

  return nullptr;
}

ScriptFieldMap &ScriptEngine::GetScriptFieldMap(Entity entity) {
  CE_API_ASSERT(entity, "Invalid Entity!");
  return GetScriptFieldMap(entity.GetUUID());
}

ScriptFieldMap &ScriptEngine::GetScriptFieldMap(UUID uuid) {
  CE_API_ASSERT(uuid, "Invalid Entity!");
  return s_ScriptData->EntityScriptFields[uuid];
}

UUID ScriptEngine::GetUUIDFromStringHash(const std::string &nameSpace,
                                         const std::string &className,
                                         const std::string &fieldName) {
  std::string combined;
  if (!nameSpace.empty() && !className.empty())
    combined = nameSpace + "::" + className + "::" + fieldName;
  else if (!nameSpace.empty())
    combined = nameSpace + "::" + fieldName;
  else if (!className.empty())
    combined = className + "::" + fieldName;

  if (s_ScriptData->StoredFieldIDs.contains(combined))
    return s_ScriptData->StoredFieldIDs[combined];

  UUID newID(Hash::GenerateFNVHash(combined));
  s_ScriptData->StoredFieldIDs[combined] = newID;
  return newID;
}

ScriptEngine &ScriptEngine::GetMutable() {
  static ScriptEngine s_Instance;
  return s_Instance;
}

void ScriptEngine::InitCapy() {
  CapyDomain *rootDomain = capy_init();

  capy_set_core_bin_include_path(Application::Get().GetMainPath() /
                                 "CatScriptCore/src");

  capy_domain_library_open(Application::Get().GetMainPath() /
                               "build/CatScriptCore/libCatScriptCore.so",
                           true);

  std::vector<CapyString> ignoredNamespaces = {
      capy_string_literal("spdlog"), capy_string_literal("fmt"),
      capy_string_literal("literals"), capy_string_literal("internal")};

  capy_set_ignored_namespace(ignoredNamespaces);

  if (rootDomain)
    s_ScriptData->RootDomain = rootDomain;
  else
    CE_API_ERROR("Could not load Jit!");
}

void ScriptEngine::ShutdownCapy() { capy_shutdown(); }

void ScriptEngine::LoadBinaryClasses() {
  s_ScriptData->EntityClasses.clear();
  for (auto &[_, lib] : s_ScriptData->RootDomain->Libraries) {
    if (lib->IsCore)
      continue;

    const CapyTableInfo *typeDefinitionsTable =
        capy_table_info_get(lib->Image.get(), CapyTableType::TypeDef);

    for (auto &[classHash, classMap] : typeDefinitionsTable->Symbols) {
      const char *nameSpace = classMap.at(0).Namespace.c_str();
      const char *name = classMap.at(0).ClassName.c_str();
      std::string fullName;
      if (strlen(nameSpace) != 0)
        fullName = fmt::format("{}.{}", nameSpace, name);
      else
        fullName = name;

      CE_API_TRACE(fullName);

      CapyClass *capyClass =
          capy_class_from_name(s_ScriptData->AppBinaryImage, nameSpace, name);

      Ref<ScriptClass> scriptClass = CreateRef<ScriptClass>(nameSpace, name);

      s_ScriptData
          ->EntityClasses[Hash::GenerateFNVHash(capyClass->ClassName.c_str())] =
          scriptClass;

      std::vector<CapyField *> fields = capy_fields_from_class(capyClass);

      for (auto *fld : fields) {
        const char *fieldName = fld->Name.c_str();

        DataType type = s_DataTypeLookup[fld->FieldTypeString.c_str()];
        scriptClass->m_Fields[Hash::GenerateFNVHash(fieldName)] = {
            type, fieldName, fld};
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////
// Script Class ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

ScriptClass::ScriptClass(const char *nameSpace, const char *className)
    : m_ClassName(className), m_ClassNamespace(nameSpace) {
  m_CapyClass =
      capy_class_from_name(s_ScriptData->AppBinaryImage, nameSpace, className);
  CE_API_ASSERT(m_CapyClass, "Class is null!");
}

CapyObject *ScriptClass::Instantiate(UUID entityID) {
  CE_API_ASSERT(m_CapyClass, "Class is null!");
  return capy_instantiate_object(m_CapyClass);
}

CapyMethod *ScriptClass::GetMethod(const char *methodName) {
  CE_API_ASSERT(m_CapyClass, "Class is null!");
  return capy_method_from_class(m_CapyClass, methodName);
}

void *ScriptClass::InvokeMethod(CapyObject *instance, CapyMethod *m,
                                const std::vector<RuntimeValue> &values) {
  CE_API_ASSERT(m_CapyClass, "Class is null!");
  std::vector<RuntimeValue> vals;
  vals = values;
  vals.insert(vals.begin(), instance->Memory);
  return capy_function_call_from_method(m, vals);
}

////////////////////////////////////////////////////////////////////////////
// Script Instance /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity)
    : m_ScriptClass(scriptClass) {
  m_Instance = scriptClass->Instantiate(entity.GetUUID());
  CE_ASSERT(m_Instance);

  for (auto &[fieldID, field] : scriptClass->GetFields()) {
    size_t valueSize = TypeToSize(field.Type);
    std::vector<uint8_t> buffer(valueSize);

    capy_field_data_get(m_Instance, m_ScriptClass->m_CapyClass, field.Name,
                        buffer.data());

    m_DefaultFieldDatas[field.Name] = std::move(buffer);
  }

  uint64_t data = entity.GetUUID();
  SetFieldData(".m_EntityID", data);

  m_StartMethod = scriptClass->GetMethod("Start");
  m_UpdateMethod = scriptClass->GetMethod("Update");

  m_CollisionEnterMethod = scriptClass->GetMethod("OnCollisionEnter");
  m_CollisionExitMethod = scriptClass->GetMethod("OnCollisionExit");
}

ScriptInstance::~ScriptInstance() {
  if (!m_Instance)
    return;

  // Restore default field data safely
  for (auto &[name, buffer] : m_DefaultFieldDatas) {
    SetFieldData(name, buffer.data());
  }

  m_DefaultFieldDatas.clear();
}

void ScriptInstance::InvokeUpdateMethod(float ts) {
  if (m_UpdateMethod)
    m_ScriptClass->InvokeMethod(m_Instance, m_UpdateMethod, {ts});
}

void ScriptInstance::InvokeStartMethod() {
  if (m_StartMethod)
    m_ScriptClass->InvokeMethod(m_Instance, m_StartMethod, {});
}
void ScriptInstance::InvokeOnCollisionEnter(const UUID &other) {
  if (m_CollisionEnterMethod)
    m_ScriptClass->InvokeMethod(m_Instance, m_CollisionEnterMethod,
                                {(uint64_t)other.uuid()});
}

void ScriptInstance::InvokeOnCollisionExit(const UUID &other) {
  if (m_CollisionExitMethod)
    m_ScriptClass->InvokeMethod(m_Instance, m_CollisionExitMethod,
                                {(uint64_t)other.uuid()});
}

bool ScriptInstance::GetFieldDataInternal(const std::string &name,
                                          void *buffer) {
  capy_field_data_get(m_Instance, m_ScriptClass->m_CapyClass, name, buffer);
  if (!buffer)
    return false;

  return true;
}

void ScriptInstance::SetFieldDataInternal(const std::string &name,
                                          void *value) {
  capy_field_data_set(m_Instance, m_ScriptClass->m_CapyClass, name, value);
}
} // namespace CatEngine
