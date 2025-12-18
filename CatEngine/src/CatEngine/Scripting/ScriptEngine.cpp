#include "cepch.h"
#include "ScriptEngine.h"

#include <capybara/capybara.h>

#include "CatEngine/Scene/Scene.h"
#include "CatEngine/Scene/Entity.h"

#include <glm/glm.hpp>

#include "CatEngine/Core/Application.h"

#include "CatEngine/Scene/Components/Components.h"

#include "CatEngine/Project/Project.h"
#include "CatEngine/Scripting/ScriptGlue.h"

#include "ScriptInstance.h"

uint64_t PortableHash(const std::string& str)
{
    uint64_t h = 0x123456789abcdef0ULL;  // custom seed
    for (unsigned char c : str)
    {
        h += c;                          // additive component
        h ^= (h << 7) | (h >> 57);       // xor + rotate
        h *= 0x165667919E3779F9ULL;      // mixing constant
    }

    // Final avalanche (makes small differences large)
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccdULL;
    h ^= h >> 29;
    h *= 0xc4ceb9fe1a85ec53ULL;
    h ^= h >> 32;

    return h;
}

namespace CatEngine
{
    static std::unordered_map<std::string, DataType> s_DataTypeLookup =
	{
        {"short",               DataType::Short     },
        {"int16_t",             DataType::Short     },
        {"unsigned short",      DataType::UShort    },
        {"uint16_t",            DataType::UShort    },
        {"int",                 DataType::Int       },
        {"int32_t",             DataType::Int       },
        {"unsigned int",        DataType::UInt      },
        {"uint32_t",            DataType::UInt      },
        {"long",                DataType::Long      },
        {"int64_t",             DataType::Long      },
        {"unsigned long",       DataType::ULong     },
        {"uint64_t",            DataType::ULong     },

        {"float",               DataType::Float     },
        {"double",              DataType::Double    },

        {"bool",                DataType::Bool      },

        {"Vector2",             DataType::Vector2   },
        {"Vector3",             DataType::Vector3   },
        {"Vector4",             DataType::Vector4   },

        {"Object",              DataType::Entity    },

        {"Texture2D",           DataType::Texture2D },
	};

	struct ScriptEngineData
	{
        std::filesystem::path CoreLibraryPath;

		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;
		std::unordered_map<UUID, Ref<ScriptInstance>> EntityInstances;

		std::unordered_map<UUID, ScriptFieldMap> EntityScriptFields;
        std::unordered_map<std::string, UUID> StoredFieldIDs;

        CapyDomain* RootDomain;

		std::vector <std::function<void()>> MainThreadQueue;

		bool SourceFileReloadPending = false;
        bool BinaryReloadPending = false;

		Ref<Scene> SceneContext = nullptr;
        
	};

	static ScriptEngineData* s_ScriptData = nullptr;   

    void CustomFileSystemEventCallback(FileEventType type, const std::filesystem::path& path)
    {

        if (type == FileEventType::Delete)
            return;
        //s_ScriptData->ReloadTimer = Timer();

        bool isRunning = s_ScriptData->SceneContext->IsRunning();


        if (isRunning)
        {
            if (!s_ScriptData->SourceFileReloadPending)
            {
                s_ScriptData->SourceFileReloadPending = true;
                s_ScriptData->MainThreadQueue.emplace_back([]()
                {
                    ScriptEngine::GetMutable().ReloadBinaries();
                });
            }
            else if (s_ScriptData->SourceFileReloadPending)
            {
                s_ScriptData->MainThreadQueue.clear();
                s_ScriptData->MainThreadQueue.emplace_back([]()
                {
					ScriptEngine::GetMutable().ReloadBinaries();
                });
            }

        }
        else if (!isRunning)
        {
            using namespace std::chrono;
            //std::this_thread::sleep_for(500ms);
            if (!s_ScriptData->SourceFileReloadPending)
            {
                s_ScriptData->SourceFileReloadPending = true;
                Application::Get().SubmitToMainThread([path]()
                {
                    ScriptEngine::GetMutable().ReloadBinaries();
                });
            }

        }
    }

    static void InternalComponentDataSet(void* ptr, void* value)
    {
        uint64_t* idPtr = reinterpret_cast<uint64_t*>(ptr);
        *idPtr = *reinterpret_cast<uint64_t*>(value);
    }


    void ScriptEngine::Init()
	{
        s_ScriptData = new ScriptEngineData();
        s_ScriptData->RootDomain = capy_jit_init();

        capy_jit_set_core_bin_include_path(Application::Get().GetMainPath() / "CatScriptCore/src");
        capy_domain_core_library_open(Application::Get().GetMainPath() / "build/CatScriptCore/libCatScriptCore.so");

        capy_set_ignored_namespace({"spdlog", "fmt", "literals", "internal"});

	}

	void ScriptEngine::Shutdown()
	{
        s_ScriptData->EntityScriptFields.clear();
        s_ScriptData->EntityClasses.clear();
		delete s_ScriptData;

        capy_jit_shutdown();
	}

    void ScriptEngine::InitializeFileSystems()
    {
        const std::filesystem::path& path = Project::GetAssetDirectory();
        capy_jit_set_source_path(path, CustomFileSystemEventCallback, true);
        capy_jit_set_binary_path(path / ".build");
        ReloadBinaries();
    }

	void ScriptEngine::SetSceneContext(Ref<Scene> scene)
	{
		s_ScriptData->SceneContext = scene;
	}

	void ScriptEngine::OnRuntimeStop()
	{
        s_ScriptData->EntityInstances.clear();

        s_ScriptData->SceneContext = nullptr;
		
        if (s_ScriptData->SourceFileReloadPending || !s_ScriptData->MainThreadQueue.empty())
			Application::Get().SubmitToMainThread(s_ScriptData->MainThreadQueue.at(0));
	}

	void ScriptEngine::ReloadBinaries()
	{
#ifdef CE_RELEASE
		//CE_CLI_TRACE(s_ScriptData->ReloadTimer.ElapsedMillis());
#endif
		//CE_API_TRACE(s_ScriptData->ReloadTimer.ElapsedMillis());

        capy_jit_poll();

        Application::Get().SubmitToMainThread([](){
            s_ScriptData->RootDomain = capy_get_root_domain();
            
            for (auto& [uuid , lib] : s_ScriptData->RootDomain->Libraries)
            {
                if (lib->IsCore)
                    continue;
                

                CapyImage* image = lib->Image.get();
                for (auto& [classID, klass] : image->Classes)
                {
                    if (klass->ClassName.empty())
                        continue;

                    auto it = s_ScriptData->EntityClasses.find(klass->ClassName);
                    if (it != s_ScriptData->EntityClasses.end())
                        s_ScriptData->EntityClasses.erase(it);

                    s_ScriptData->EntityClasses[klass->ClassName] = CreateRef<ScriptClass>(image, klass->NameSpace, klass->ClassName);

                    auto& scriptClass = s_ScriptData->EntityClasses[klass->ClassName];

                    CapyClass* klassPtr = klass.get();
                    for (auto& [uuid, field] : klassPtr->VTable->Fields)
                    {
                        if (!field)
                            continue;

                        if (!s_DataTypeLookup.contains(field->FieldTypeString))
                            continue;

                        DataType type = s_DataTypeLookup[field->FieldTypeString];
                        UUID fieldID(ScriptEngine::GetMutable().GetUUIDFromStringHash(klass->NameSpace, field->SymbolMetaData.ClassName, field->SymbolMetaData.Name));
                        scriptClass->m_Fields[fieldID] = ScriptField{field->SymbolMetaData.Namespace, field->SymbolMetaData.ClassName, field->SymbolMetaData.Name, type};
                    }

                }
            }

            ScriptGlue::RegisterFunctions();
            ScriptGlue::RegisterComponents();

            CE_API_WARN(capy_dump_domain());

        });

        s_ScriptData->SourceFileReloadPending = false;
	}

	bool ScriptEngine::ScriptClassExists(const std::string& fullClassName)
	{
		return s_ScriptData->EntityClasses.find(fullClassName) != s_ScriptData->EntityClasses.end();
	}

	void ScriptEngine::OnStartEntity(Entity e)
	{

		const auto& sc = e.GetComponent<ScriptComponent>();

		if (ScriptEngine::ScriptClassExists(sc.ClassName))
		{
			UUID entityID = e.GetUUID();
			Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(s_ScriptData->EntityClasses[sc.ClassName], e);
			s_ScriptData->EntityInstances[entityID] = instance;

			// Copy field value if we set it
			if (s_ScriptData->EntityScriptFields.find(entityID) != s_ScriptData->EntityScriptFields.end())
			{
				const ScriptFieldMap& scriptFieldMap = s_ScriptData->EntityScriptFields.at(entityID);

				for (const auto& [uuid, fieldInstance] : scriptFieldMap)
				{
                    CE_API_WARN("Field Instance Name: {}", fieldInstance.Field.Name);
					instance->SetFieldDataInternal(fieldInstance.Field.Name, (void*)fieldInstance.m_Data.Data);
				}
			}

			instance->InvokeStartMethod();
		}
	}

	void ScriptEngine::OnUpdateEntity(Entity e, float ts)
	{
		UUID entityUUID = e.GetUUID();

		if (s_ScriptData->EntityInstances.find(entityUUID) != s_ScriptData->EntityInstances.end())
		{
			Ref<ScriptInstance> instance = s_ScriptData->EntityInstances[entityUUID];
			instance->InvokeUpdateMethod(ts);
		}
		else
		{
			CE_API_ERROR("Could not find script instance {}", (uint64_t)entityUUID);
		}

	}

    void ScriptEngine::DispatchCollisionEvent(UUID uuidA, UUID uuidB, CollisionType type)
    {
      Fields:
        Ref<ScriptInstance> scriptA, scriptB;
        if (s_ScriptData->EntityInstances.find(uuidA) != s_ScriptData->EntityInstances.end())
            scriptA = s_ScriptData->EntityInstances[uuidA];

        if (s_ScriptData->EntityInstances.find(uuidB) != s_ScriptData->EntityInstances.end())
            scriptB = s_ScriptData->EntityInstances[uuidB];


        switch (type)
        {
            case CollisionType::Begin:
                if (scriptA) scriptA->InvokeOnCollisionEnter(uuidB);
                if (scriptB) scriptB->InvokeOnCollisionEnter(uuidA);
                break;
            case CollisionType::End:
                if (scriptA) scriptA->InvokeOnCollisionExit(uuidB);
                if (scriptB) scriptB->InvokeOnCollisionExit(uuidA);
                break;
            default: break;
        }
    }

	Ref<Scene> ScriptEngine::GetSceneContext()
	{
		return s_ScriptData->SceneContext;
	}

	Ref<ScriptInstance> ScriptEngine::GetEntityScriptInstance(UUID entityID)
	{
		auto it = s_ScriptData->EntityInstances.find(entityID);
		if (it == s_ScriptData->EntityInstances.end())
			return nullptr;

		return it->second;
	}

    const ScriptFieldMap& ScriptEngine::GetInitializedFields(const UUID& entityID) const
    {
        static ScriptFieldMap s_EmptyFieldMap = {};
        if (!s_ScriptData->EntityScriptFields.contains(entityID))
            return s_EmptyFieldMap;

        return s_ScriptData->EntityScriptFields.at(entityID);
    }
    
	std::unordered_map<std::string, Ref<ScriptClass>>& ScriptEngine::GetScriptClasses()
	{
		return s_ScriptData->EntityClasses;
	}

	Ref<ScriptClass> ScriptEngine::GetScriptClass(const std::string& name)
	{
		if (s_ScriptData->EntityClasses.find(name) != s_ScriptData->EntityClasses.end())
			return s_ScriptData->EntityClasses.at(name);

		return nullptr;
	}

	ScriptFieldMap& ScriptEngine::GetScriptFieldMap(Entity entity)
	{
        CE_API_ASSERT(entity, "Invalid Entity!");
        return GetScriptFieldMap(entity.GetUUID());
    }

    ScriptFieldMap& ScriptEngine::GetScriptFieldMap(UUID uuid)
	{
        CE_API_ASSERT(uuid, "Invalid Entity!");
        return s_ScriptData->EntityScriptFields[uuid];
	}


    UUID ScriptEngine::GetUUIDFromStringHash(const std::string& nameSpace, const std::string& className, const std::string& fieldName)
    {
        std::string combined;
        if (!nameSpace.empty() && !className.empty())
            combined = nameSpace + "::" + className + "::" + fieldName;
        else if (!nameSpace.empty())
            combined = nameSpace + "::" + fieldName;
        else if (!className.empty())
            combined = className + "::" + fieldName;

        if (s_ScriptData->StoredFieldIDs.contains(combined))
            return s_ScriptData->StoredFieldIDs[combined];

        UUID newID(PortableHash(combined));
        s_ScriptData->StoredFieldIDs[combined] = newID;
        return newID;
    }
    
    ScriptEngine& ScriptEngine::GetMutable()
	{
		static ScriptEngine s_Instance;
		return s_Instance;
	}
}
