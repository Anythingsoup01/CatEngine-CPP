#include "ScriptEngine.h"
#include "CatEngine/Scripting/ScriptEntityStorage.h"
#include "cepch.h"

#include <Capybara/Core/Capybara.h>
#include <Capybara/Core/ReflectionTypes.h>

#include <Capybara/JIT/JIT.h>

#include "CatEngine/Scene/Entity.h"
#include "CatEngine/Scene/Scene.h"

#include <cstdint>
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
  std::filesystem::path CoreLibraryPath;

  std::unordered_map<uint64_t, Ref<ScriptClass>> EntityClasses;
  std::unordered_map<UUID, Ref<ScriptInstance>> EntityInstances;
  std::unordered_map<UUID, ScriptFieldMap> EntityScriptFields;

  Ref<Scene> SceneContext = nullptr;
};

static ScriptEngineData *s_ScriptData = nullptr;

////////////////////////////////////////////////////////////////////////////
// Script Engine ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

ScriptEngine::ScriptEngine() {
  if (s_Instance) {
    return;
  }
  s_Instance = this;

  s_ScriptData = new ScriptEngineData();
}

void ScriptEngine::Init() {
  ScriptGlue::RegisterComponents();
  ScriptGlue::RegisterFunctions();

  CapyInit();

  LoadBinaryClasses();
}

void ScriptEngine::Shutdown() {
  s_ScriptData->EntityScriptFields.clear();
  s_ScriptData->EntityClasses.clear();
  delete s_ScriptData;

  CapyShutdown();
}

void ScriptEngine::SetSceneContext(Ref<Scene> scene) {
  s_ScriptData->SceneContext = scene;
}

void ScriptEngine::OnRuntimeStop() {
  s_ScriptData->EntityInstances.clear();
  s_ScriptData->SceneContext = nullptr;
}

void ScriptEngine::PollJIT() {
  if (!s_ScriptData->SceneContext || s_ScriptData->SceneContext->IsRunning())
    return;

  if (!Capybara::Jit::Poll())
    return;

  Application::Get().SubmitToMainThread([this]() {
    LoadBinaryClasses();

    ScriptGlue::RegisterFunctions();
    ScriptGlue::RegisterComponents();
  });
}

bool ScriptEngine::ScriptClassExists(const std::string_view &className) {
  // TODO: Crossover event! Add namespace tracking, i.e anything before the
  // class name
  uint64_t hash = Hash::GenerateFNVHash(className.data());
  return s_ScriptData->EntityClasses.find(hash) !=
         s_ScriptData->EntityClasses.end();
}

void ScriptEngine::OnStartEntity(Entity e) {

  const auto &sc = e.GetComponent<ScriptComponent>();

  if (ScriptEngine::ScriptClassExists(sc.ClassName.c_str())) {
    UUID entityID = e.GetUUID();

    Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(s_ScriptData->EntityClasses[Hash::GenerateFNVHash(sc.ClassName)], e);

    if (!instance)
      return;

    s_ScriptData->EntityInstances[entityID] = instance;

    CE_API_INFO(e.GetName());

    // Copy field value if we set it
    if (s_ScriptData->EntityScriptFields.find(entityID) !=
        s_ScriptData->EntityScriptFields.end()) {
      const ScriptFieldMap &scriptFieldMap =
          s_ScriptData->EntityScriptFields.at(entityID);

      for (const auto &[entityID, scriptHash] : scriptFieldMap) {
        for (const auto &[fieldHash, fieldInstance] : scriptHash) {
          CE_API_WARN("Field Instance Name: {}", fieldInstance.Field.Name);
          instance->SetFieldDataInternal(fieldInstance.Field.Name, fieldInstance.Data.Data, fieldInstance.Field.Size);
        }
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

void ScriptEngine::DispatchCollisionEvent(UUID uuidA, UUID uuidB, CollisionType type) {
  Ref<ScriptInstance> scriptA, scriptB;
  if (s_ScriptData->EntityInstances.find(uuidA) != s_ScriptData->EntityInstances.end())
    scriptA = s_ScriptData->EntityInstances[uuidA];

  if (s_ScriptData->EntityInstances.find(uuidB) != s_ScriptData->EntityInstances.end())
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

const ScriptFieldMap &
ScriptEngine::GetInitializedFields(const UUID &entityID) const {
  static ScriptFieldMap s_EmptyFieldMap = {};
  if (s_ScriptData->EntityScriptFields.find(entityID) ==
      s_ScriptData->EntityScriptFields.end())
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

void ScriptEngine::CapyInit() {
  Capybara::Capybara::Init();

  Capybara::Jit::SetBuildDirectory(Project::GetAssetDirectory() / ".capy");
  Capybara::Capybara::SetExternalLibrary(Application::Get().GetMainPath() / "build/CatScriptCore/libCatScriptCore.so", Application::Get().GetMainPath() / "CatScriptCore/src");
  Capybara::Jit::Init(Project::GetAssetDirectory());
  Capybara::Capybara::LoadLibrary(Project::GetAssetDirectory() / ".capy/CapyRuntimeLib.so");

}

void ScriptEngine::CapyShutdown() { Capybara::Capybara::Shutdown(); }

void ScriptEngine::LoadBinaryClasses() {
  s_ScriptData->EntityClasses.clear();
  Capybara::Capybara::ReloadLibrary(Project::GetAssetDirectory() / ".capy/CapyRuntimeLib.so");

  const auto &capyClasses = Capybara::Capybara::GetClasses();


  for (auto &[className, classObj] : capyClasses) {
    if (classObj->Visibility == Capybara::MemberVisibility::Private || classObj->Visibility == Capybara::MemberVisibility::Protected)
      continue;
    Ref<ScriptClass> scriptClass = CreateRef<ScriptClass>(className);
    uint32_t scriptHash = Hash::GenerateFNVHash(className);

    // 1. Populate scriptClass fields ordered/mapped by ByteOffset for execution/layout
    for (auto it = classObj->Fields.rbegin(); it != classObj->Fields.rend(); ++it) {
      auto field = *it;
      if (field.Visibility == Capybara::MemberVisibility::Private || field.Visibility == Capybara::MemberVisibility::Protected)
        continue;
      DataType type = s_DataTypeLookup.at(field.TypeName);
      std::string name = field.Name;
      uint64_t fieldHash = Hash::GenerateFNVHash(field.Name);

      scriptClass->m_Fields[fieldHash] = {name, type, field.SizeInBytes, field.DefaultValue};
    }

    s_ScriptData->EntityClasses.emplace(scriptHash, scriptClass);
  }
  // 2. Clean up and synchronize entity field instances across all entities
  for (auto entityIt = s_ScriptData->EntityScriptFields.begin(); entityIt != s_ScriptData->EntityScriptFields.end(); ) {
    auto &[entityUUID, scriptFieldMap] = *entityIt;

    for (auto scriptIt = scriptFieldMap.begin(); scriptIt != scriptFieldMap.end(); ) {
      uint64_t scriptHash = scriptIt->first;
      auto &fieldInstanceMap = scriptIt->second;

      // If the script class no longer exists at all, drop its field entries
      if (s_ScriptData->EntityClasses.find(scriptHash) == s_ScriptData->EntityClasses.end()) {
        scriptIt = scriptFieldMap.erase(scriptIt);
        continue;
      }

      Ref<ScriptClass> scriptClass = s_ScriptData->EntityClasses.at(scriptHash);

      // Build a set of currently valid field hashes for this script class
      std::unordered_set<uint64_t> validFieldHashes;
      for (const auto &[fieldID, field] : scriptClass->m_Fields) {
        validFieldHashes.insert(fieldID);
      }

      // Prune any stored field instance that no longer exists in the compiled code
      for (auto fieldIt = fieldInstanceMap.begin(); fieldIt != fieldInstanceMap.end(); ) {
        uint64_t fieldHash = fieldIt->first;
        if (validFieldHashes.find(fieldHash) == validFieldHashes.end()) {
          fieldIt = fieldInstanceMap.erase(fieldIt); // Field was deleted or renamed!
        } else {
          ++fieldIt;
        }
      }

      // Clean up empty script maps if no instances remain
      if (fieldInstanceMap.empty()) {
        scriptIt = scriptFieldMap.erase(scriptIt);
      } else {
        ++scriptIt;
      }
    }

    // Clean up entity entry if it has no active scripts left
    if (scriptFieldMap.empty()) {
      entityIt = s_ScriptData->EntityScriptFields.erase(entityIt);
    } else {
      ++entityIt;
    }
  }
}

////////////////////////////////////////////////////////////////////////////
// Script Class ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

ScriptClass::ScriptClass(const std::string &className)
    : m_ClassName(className) {
  m_ClassObject = Capybara::Capybara::GetClass(className);
  CE_API_ASSERT(m_ClassObject, "Class is null!");
}

Ref<Capybara::CapyObject::Instance> ScriptClass::Instantiate(UUID entityID) {
  CE_API_ASSERT(m_ClassObject, "Class is null!");
  auto instance = m_ClassObject->CreateInstance();
  instance->SetField("m_EntityID", entityID.uuid(), m_ClassObject.get());
  return instance;
}

////////////////////////////////////////////////////////////////////////////
// Script Instance /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity)
    : m_ScriptClass(scriptClass) {
  m_Instance = scriptClass->Instantiate(entity.GetUUID());
  CE_ASSERT(m_Instance);
}

ScriptInstance::~ScriptInstance() {
  if (!m_Instance)
    return;
}

void ScriptInstance::InvokeUpdateMethod(float ts) {
  m_ScriptClass->InvokeMethod<void>("Update", m_Instance, ts);
}

void ScriptInstance::InvokeStartMethod() {
  m_ScriptClass->InvokeMethod<void>("Start", m_Instance);
}

void ScriptInstance::InvokeOnCollisionEnter(const UUID &other) {
  m_ScriptClass->InvokeMethod<void>("OnCollisionEnter", m_Instance, (uint64_t)other.uuid());
}

void ScriptInstance::InvokeOnCollisionExit(const UUID &other) {
  m_ScriptClass->InvokeMethod<void>("OnCollisionExit", m_Instance, (uint64_t)other.uuid());
}

} // namespace CatEngine
