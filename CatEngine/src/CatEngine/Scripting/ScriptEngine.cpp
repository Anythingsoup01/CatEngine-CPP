#include "ScriptEngine.h"
#include "CatEngine/Editor/PanelManager.h"
#include "CatEngine/Editor/Panels/ConsolePanel.h"
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

    {"vector", DataType::DynamicVector},

    {"Object", DataType::Entity},

    {"Texture2D", DataType::Texture2D},
};

struct ScriptEngineData {
  std::filesystem::path CoreLibraryPath;

  std::unordered_map<UUID, Ref<ScriptClass>> EntityClasses;
  std::unordered_map<UUID, EntityScriptInstanceMap> EntityInstances;
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

void ScriptEngine::LoadRuntimeScripts() {
  auto view =
      s_ScriptData->SceneContext->GetReg().view<ScriptComponentContainer>();

  // Instantiate All Scripts
  for (auto entityID : view) {
    Entity e = {entityID, s_ScriptData->SceneContext.get()};
    auto &scc = e.GetComponent<ScriptComponentContainer>();

    for (auto &sc : scc.Scripts) {
      if (!ScriptEngine::ScriptClassExists(sc.ClassName.c_str()))
        continue;

      UUID entUUID = e.GetUUID();

      // Create the script instance and register it immediately
      // so global lookups/registries can see that it exists in memory.
      Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(
          s_ScriptData->EntityClasses[Hash::GenerateFNVHash(sc.ClassName)], e);

      if (!instance)
        continue;

      s_ScriptData->EntityInstances[entUUID][sc.ScriptID] = instance;
    }
  }

  for (auto entityID : view) {
    Entity e = {entityID, s_ScriptData->SceneContext.get()};
    auto &scc = e.GetComponent<ScriptComponentContainer>();
    UUID entUUID = e.GetUUID();

    if (s_ScriptData->EntityInstances.find(entUUID) ==
        s_ScriptData->EntityInstances.end())
      continue;

    auto &scriptInstances = s_ScriptData->EntityInstances[entUUID];

    for (auto &sc : scc.Scripts) {
      if (scriptInstances.find(sc.ScriptID) == scriptInstances.end())
        continue;

      auto &instance = scriptInstances[sc.ScriptID];

      // Copy saved field values into the instance memory
      if (s_ScriptData->EntityScriptFields.find(entUUID) !=
          s_ScriptData->EntityScriptFields.end()) {
        auto &scriptFieldMap = s_ScriptData->EntityScriptFields.at(entUUID);

        if (scriptFieldMap.find(sc.ScriptID) != scriptFieldMap.end()) {
          auto &fieldMap = scriptFieldMap.at(sc.ScriptID);
          for (auto &[fieldHash, fieldInstance] : fieldMap) {
            if (fieldInstance.Field.IsVector) {
              // Pass the editor's serialized VectorBuffer blob into the
              // instance initializer
              if (fieldInstance.Field.ElementType ==
                  DataType::ScriptComponent) {
                // 1. Copy the VectorData object so we don't corrupt
                // s_ScriptData->EntityScriptFields
                auto runtimeVectorData = fieldInstance.VectorData;

                for (size_t i = 0; i < runtimeVectorData.GetCount(); i++) {
                  void *elementPtr = runtimeVectorData.GetElementPointer(
                      i, fieldInstance.Field.ElementSize);
                  if (!elementPtr)
                    continue;

                  // Read the persistent Entity ID from our local copy
                  uint64_t targetEntityID =
                      *reinterpret_cast<uint64_t *>(elementPtr);
                  if (targetEntityID == 0)
                    continue;

                  uint64_t classHash = Hash::GenerateFNVHash(
                      fieldInstance.Field.ElementTypeName);
                  Ref<ScriptInstance> otherInstance =
                      GetEntityScriptInstance(targetEntityID, classHash);

                  if (!otherInstance)
                    continue;

                  // Write the runtime pointer into our local copy's slot
                  void **slotPtr = reinterpret_cast<void **>(elementPtr);
                  *slotPtr =
                      otherInstance->GetManagedObject()->RawMemory.data();
                }

                // 2. Pass the modified runtime copy to the instance initializer
                instance->m_Instance->InitializeVectorField(
                    fieldInstance.Field.Name,
                    instance->m_ScriptClass->m_ClassObject.get(),
                    runtimeVectorData.Data, fieldInstance.Field.ElementSize);
                continue;
              }
            }
            if (fieldInstance.Field.Type != DataType::ScriptComponent) {
              instance->SetFieldDataInternal(
                  fieldInstance.Field.Name,
                  (void *)fieldInstance.TrivialData.Data,
                  fieldInstance.Field.Size);
            } else {
              uint64_t classHash =
                  Hash::GenerateFNVHash(fieldInstance.Field.TypeName);
              uint64_t targetEntityID =
                  *(uint64_t *)fieldInstance.TrivialData.Data;
              Ref<ScriptInstance> otherInstance =
                  GetEntityScriptInstance(targetEntityID, classHash);
              if (!otherInstance)
                continue;

              void *data = otherInstance->GetManagedObject()->RawMemory.data();
              instance->SetFieldDataInternal(fieldInstance.Field.Name, data,
                                             fieldInstance.Field.Size);
            }
          }
        }
      }
    }
  }
  for (auto entityID : view) {
    Entity e = {entityID, s_ScriptData->SceneContext.get()};
    auto &scc = e.GetComponent<ScriptComponentContainer>();
    UUID entUUID = e.GetUUID();

    if (s_ScriptData->EntityInstances.find(entUUID) ==
        s_ScriptData->EntityInstances.end())
      continue;

    auto &scriptInstances = s_ScriptData->EntityInstances[entUUID];

    for (auto &sc : scc.Scripts) {
      if (scriptInstances.find(sc.ScriptID) == scriptInstances.end())
        continue;

      auto &instance = scriptInstances[sc.ScriptID];
      instance->InvokeStartMethod();
    }
  }
}

void ScriptEngine::OnUpdateEntity(Entity e, float ts) {
  auto &scc = e.GetComponent<ScriptComponentContainer>();
  UUID entityUUID = e.GetUUID();
  for (auto &sc : scc.Scripts) {
    auto &instances = s_ScriptData->EntityInstances;
    if (instances.find(entityUUID) == instances.end()) {
      PanelManager::Get()
          .GetPanel<ConsolePanel>("ConsolePanel")
          ->WriteCritical("Failed to find script instance " +
                          std::to_string(entityUUID));
      CE_API_ERROR("Could not find script instance {}", (uint64_t)entityUUID);
      continue;
    }

    auto entityInstances = instances.at(entityUUID);
    for (auto &[scriptID, sc] : entityInstances) {
      Ref<ScriptInstance> instance = entityInstances[scriptID];
      instance->InvokeUpdateMethod(ts);
    }
  }
}

void ScriptEngine::DispatchCollisionEvent(UUID uuidA, UUID uuidB,
                                          CollisionType type) {
  auto &instances = s_ScriptData->EntityInstances;
  EntityScriptInstanceMap entityA, entityB;
  if (instances.find(uuidA) != instances.end())
    entityA = instances[uuidA];

  if (instances.find(uuidB) != instances.end())
    entityB = instances[uuidB];

  for (auto &[scriptID, script] : entityA) {
    switch (type) {
    case CollisionType::Begin:
      script->InvokeOnCollisionEnter(uuidB);
      break;
    case CollisionType::End:
      if (script)
        script->InvokeOnCollisionExit(uuidB);
      break;
    default:
      break;
    }
  }

  for (auto &[scriptID, script] : entityB) {
    switch (type) {
    case CollisionType::Begin:
      script->InvokeOnCollisionEnter(uuidA);
      break;
    case CollisionType::End:
      if (script)
        script->InvokeOnCollisionExit(uuidA);
      break;
    default:
      break;
    }
  }
}

Ref<Scene> ScriptEngine::GetSceneContext() {
  return s_ScriptData->SceneContext;
}

Ref<ScriptInstance>
ScriptEngine::GetEntityScriptInstance(const UUID &entityID,
                                      const UUID &scriptID) {
  auto &instances = s_ScriptData->EntityInstances;
  if (instances.find(entityID) == instances.end()) {
    return nullptr;
  }

  auto &entityScriptInstances = instances[entityID];
  if (entityScriptInstances.find(scriptID) == entityScriptInstances.end()) {
    return nullptr;
  }

  return entityScriptInstances[scriptID];
}

const FieldMap &ScriptEngine::GetInitializedFields(const UUID &entityID,
                                                   const UUID &scriptID) const {
  static FieldMap s_EmptyFieldMap = {};
  auto &instances = s_ScriptData->EntityScriptFields;
  if (instances.find(entityID) == instances.end()) {
    return s_EmptyFieldMap;
  }

  auto &entityScriptInstances = instances[entityID];
  if (entityScriptInstances.find(scriptID) == entityScriptInstances.end()) {
    return s_EmptyFieldMap;
  }

  return entityScriptInstances[scriptID];
}

std::unordered_map<UUID, Ref<ScriptClass>> &ScriptEngine::GetScriptClasses() {
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
  Capybara::Capybara::SetExternalLibrary(
      Application::Get().GetMainPath() /
          "build/CatScriptCore/libCatScriptCore.so",
      Application::Get().GetMainPath() / "CatScriptCore/src");
  Capybara::Jit::Init(Project::GetAssetDirectory());
  Capybara::Capybara::LoadLibrary(Project::GetAssetDirectory() /
                                  ".capy/CapyRuntimeLib.so");
}

void ScriptEngine::CapyShutdown() { Capybara::Capybara::Shutdown(); }

void ScriptEngine::LoadBinaryClasses() {
  s_ScriptData->EntityClasses.clear();
  Capybara::Capybara::ReloadLibrary(Project::GetAssetDirectory() /
                                    ".capy/CapyRuntimeLib.so");

  const auto &capyClasses = Capybara::Capybara::GetClasses();

  for (auto &[className, classObj] : capyClasses) {
    if (classObj->Visibility == Capybara::MemberVisibility::Private ||
        classObj->Visibility == Capybara::MemberVisibility::Protected)
      continue;
    Ref<ScriptClass> scriptClass = CreateRef<ScriptClass>(className);
    uint32_t scriptHash = Hash::GenerateFNVHash(className);

    // 1. Populate scriptClass fields ordered/mapped by ByteOffset for
    // execution/layout
    for (auto it = classObj->Fields.rbegin(); it != classObj->Fields.rend();
         ++it) {
      auto field = *it;
      if (field.Visibility == Capybara::MemberVisibility::Private ||
          field.Visibility == Capybara::MemberVisibility::Protected)
        continue;
      // Assume any data that isn't apart of the listed components is a script
      // class name
      DataType type = DataType::ScriptComponent;
      if (s_DataTypeLookup.find(field.TypeName) != s_DataTypeLookup.end()) {
        type = s_DataTypeLookup.at(field.TypeName);
      }

      DataType elementType = DataType::ScriptComponent;
      if (field.IsVector && (s_DataTypeLookup.find(field.ElementTypeName) !=
                             s_DataTypeLookup.end())) {
        elementType = s_DataTypeLookup.at(field.ElementTypeName);
      }

      std::string name = field.Name;
      auto trimWhitespace = [](std::string &str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos) {
          str.clear();
          return;
        }
        size_t last = str.find_last_not_of(" \t\n\r");
        str = str.substr(first, (last - first + 1));
      };

      // 1. Clean field.TypeName
      if (field.TypeName.find('*') != std::string::npos) {
        field.TypeName = field.TypeName.substr(0, field.TypeName.find('*'));
        trimWhitespace(field.TypeName);
      }

      // 2. Clean field.ElementTypeName
      if (!field.ElementTypeName.empty() &&
          field.ElementTypeName.find('*') != std::string::npos) {
        field.ElementTypeName =
            field.ElementTypeName.substr(0, field.ElementTypeName.find('*'));
        trimWhitespace(field.ElementTypeName);
      }

      uint64_t fieldHash = Hash::GenerateFNVHash(field.Name);

      ScriptField f = {name,
                       type,
                       field.TypeName,
                       field.SizeInBytes,
                       field.IsVector,
                       elementType,
                       field.ElementTypeName,
                       field.ElementSizeInBytes,
                       field.ElementCount,
                       field.DefaultValue};

      scriptClass->m_Fields[fieldHash] = f;
    }

    s_ScriptData->EntityClasses.emplace(scriptHash, scriptClass);
  }
  // 2. Clean up and synchronize entity field instances across all entities
  for (auto entityIt = s_ScriptData->EntityScriptFields.begin();
       entityIt != s_ScriptData->EntityScriptFields.end();) {
    auto &[entityUUID, scriptFieldMap] = *entityIt;

    for (auto scriptIt = scriptFieldMap.begin();
         scriptIt != scriptFieldMap.end();) {
      uint64_t scriptHash = scriptIt->first;
      auto &fieldInstanceMap = scriptIt->second;

      // If the script class no longer exists at all, drop its field entries
      if (s_ScriptData->EntityClasses.find(scriptHash) ==
          s_ScriptData->EntityClasses.end()) {
        scriptIt = scriptFieldMap.erase(scriptIt);
        continue;
      }

      Ref<ScriptClass> scriptClass = s_ScriptData->EntityClasses.at(scriptHash);

      // Build a set of currently valid field hashes for this script class
      std::unordered_set<uint64_t> validFieldHashes;
      for (const auto &[fieldID, field] : scriptClass->m_Fields) {
        validFieldHashes.insert(fieldID);
      }

      // Prune any stored field instance that no longer exists in the compiled
      // code
      for (auto fieldIt = fieldInstanceMap.begin();
           fieldIt != fieldInstanceMap.end();) {
        uint64_t fieldHash = fieldIt->first;
        if (validFieldHashes.find(fieldHash) == validFieldHashes.end()) {
          fieldIt =
              fieldInstanceMap.erase(fieldIt); // Field was deleted or renamed!
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

  for (auto &method : m_ClassObject->Methods) {
    if (method.Name == "Start") {
      m_HasStart = true;
    } else if (method.Name == "Update") {
      m_HasUpdate = true;
    } else if (method.Name == "OnCollisionEnter") {
      m_HasOnCollisionEnter = true;
    } else if (method.Name == "OnCollisionExit") {
      m_HasOnCollisionExit = true;
    }
  }
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
  if (m_ScriptClass->m_HasUpdate)
    m_ScriptClass->InvokeMethod<void>("Update", m_Instance, ts);
}

void ScriptInstance::InvokeStartMethod() {
  if (m_ScriptClass->m_HasStart)
    m_ScriptClass->InvokeMethod<void>("Start", m_Instance);
}

void ScriptInstance::InvokeOnCollisionEnter(const UUID &other) {
  if (m_ScriptClass->m_HasOnCollisionEnter)
    m_ScriptClass->InvokeMethod<void>("OnCollisionEnter", m_Instance,
                                      (uint64_t)other.uuid());
}

void ScriptInstance::InvokeOnCollisionExit(const UUID &other) {
  if (m_ScriptClass->m_HasOnCollisionExit)
    m_ScriptClass->InvokeMethod<void>("OnCollisionExit", m_Instance,
                                      (uint64_t)other.uuid());
}

} // namespace CatEngine
