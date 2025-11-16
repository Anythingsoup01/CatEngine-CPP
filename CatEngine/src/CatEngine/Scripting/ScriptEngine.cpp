#include "cepch.h"
#include "ScriptEngine.h"

#include "FileWatch.hpp"

#include <Capybara/Capybara.h>

#include "CatEngine/Scene/Scene.h"
#include "CatEngine/Scene/Entity.h"


#include <glm/glm.hpp>
#include <unordered_map>


#include "CatEngine/Core/Application.h"

#include "CatEngine/Scene/Components/Components.h"

#include "CatEngine/Project/Project.h"
#include "CatEngine/Scripting/ScriptGlue.h"

static bool s_ReloadFileWatcher = false;

namespace CatEngine
{
	struct ScriptEngineData
	{
        CapyDomain* AppDomain;
        std::filesystem::path CoreLibraryPath;

		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;
		std::unordered_map<UUID, Ref<ScriptInstance>> EntityInstances;

		std::unordered_map<UUID, ScriptFieldMap> EntityScriptFields;

        std::vector<Scope<filewatch::FileWatch<std::string>>> SourceFileWatchers;
        Scope<filewatch::FileWatch<std::string>> CompiledScriptsFileWatcher;

		std::vector <std::function<void()>> MainThreadQueue;

		bool SourceFileReloadPending = false;
        bool BinaryReloadPending = false;

		Ref<Scene> SceneContext = nullptr;

	};

	static ScriptEngineData* s_ScriptData = nullptr;   

	///////////////////////////////////////////////////////
	// SCRIPT ENGINE //////////////////////////////////////
	///////////////////////////////////////////////////////
    void ScriptEngine::Init()
	{
        capy_init();

		s_ScriptData = new ScriptEngineData();
        s_ScriptData->AppDomain = capy_init_domain("CatAppDomain");

        capy_set_ignored_namespace({"spdlog", "fmt", "literals", "internal"});

        s_ScriptData->CoreLibraryPath = Application::Get().GetMainPath() / "build/CatScriptCore/libCatScriptCore.so";

        capy_domain_library_open(s_ScriptData->AppDomain, s_ScriptData->CoreLibraryPath.c_str(), true);

	}

	void ScriptEngine::Shutdown()
	{
        s_ScriptData->EntityScriptFields.clear();
        s_ScriptData->SourceFileWatchers.clear();
        s_ScriptData->EntityClasses.clear();
		delete s_ScriptData;

        capy_shutdown();
	}

	static void OnSourceFileSystemEvent(const std::string& path, const filewatch::Event change_type)
	{
		
		if (change_type == filewatch::Event::modified)
		{
			bool isRunning = s_ScriptData->SceneContext->IsRunning();

			//s_ScriptData->ReloadTimer = Timer();

			if (isRunning)
			{
				if (!s_ScriptData->SourceFileReloadPending)
				{
					s_ScriptData->SourceFileReloadPending = true;
					s_ScriptData->MainThreadQueue.emplace_back([]()
					{
                        for (auto& fileWatcher : s_ScriptData->SourceFileWatchers)
                            fileWatcher.reset();
						ScriptEngine::ReloadBinaries();
					});
				}
				else if (s_ScriptData->SourceFileReloadPending)
				{
					s_ScriptData->MainThreadQueue.clear();
					s_ScriptData->MainThreadQueue.emplace_back([]()
					{
                        for (auto& fileWatcher : s_ScriptData->SourceFileWatchers)
                            fileWatcher.reset();
						ScriptEngine::ReloadBinaries();
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
                        for (auto& fileWatcher : s_ScriptData->SourceFileWatchers)
                            fileWatcher.reset();
                        s_ReloadFileWatcher = true;
                        SourceFileCompiler::AddFile(path);
						ScriptEngine::ReloadBinaries();
					});
				}

			}

		}


	}

    void ScriptEngine::InitializeFileSystems()
    {
        const std::filesystem::path& path = Project::GetAssetDirectory();
        LoadFileWatcher(path);
        capy_set_libraries_path(path / ".build");
        ReloadBinaries();
    }

	bool ScriptEngine::LoadFileWatcher(const std::filesystem::path& filePath)
	{
        for (const auto& entry : std::filesystem::recursive_directory_iterator(filePath)) 
        {
            std::string path = entry.path().string();
            bool hiddenPath = path.find(".") != std::string::npos;
            if (entry.is_directory() && !hiddenPath)
            {
                s_ScriptData->SourceFileWatchers.push_back(CreateScope<filewatch::FileWatch<std::string>>(
			entry.path().string(), OnSourceFileSystemEvent));

            }
        }

        s_ScriptData->SourceFileWatchers.push_back(CreateScope<filewatch::FileWatch<std::string>>(filePath.string(), OnSourceFileSystemEvent));

		return true;
	}

	void ScriptEngine::SetSceneContext(Ref<Scene> scene)
	{
		s_ScriptData->SceneContext = scene;
	}

	void ScriptEngine::OnRuntimeStop()
	{
        if (s_ScriptData->SceneContext) 
        {
            // 1) Destroy all script instances while scene is valid
            s_ScriptData->EntityInstances.clear();

            // 2) Optionally clear field data too
            s_ScriptData->EntityScriptFields.clear();

            // 3) SceneContext can be safely null now
            s_ScriptData->SceneContext = nullptr;
        }		
		if (s_ScriptData->SourceFileReloadPending || !s_ScriptData->MainThreadQueue.empty())
			Application::Get().SubmitToMainThread(s_ScriptData->MainThreadQueue.at(0));
	}

	void ScriptEngine::ReloadBinaries()
	{
#ifdef CE_RELEASE
		//CE_CLI_TRACE(s_ScriptData->ReloadTimer.ElapsedMillis());
#endif
		//CE_API_TRACE(s_ScriptData->ReloadTimer.ElapsedMillis());

        capy_unload_domain("CatAppDomain");

        Application::Get().SubmitToMainThread([](){
            s_ScriptData->AppDomain = capy_init_domain("CatAppDomain");

            capy_domain_library_open(s_ScriptData->AppDomain, s_ScriptData->CoreLibraryPath.c_str(), true);

            capy_reload_libraries_into_domain(s_ScriptData->AppDomain);

            

            for (auto& [name , lib] : s_ScriptData->AppDomain->Libraries)
            {
                if (lib->IsCore)
                    continue;
                

                CapyImage* image = lib->MainImage.get();
                for (auto& [name, klass] : image->Classes)
                {
                    auto it = s_ScriptData->EntityClasses.find(klass->ClassName);
                    if (it != s_ScriptData->EntityClasses.end())
                        s_ScriptData->EntityClasses.erase(it);

                    s_ScriptData->EntityClasses[klass->ClassName] = CreateRef<ScriptClass>(image, klass->NameSpace, klass->ClassName);


                    CapyClass* klassPtr = klass.get();
                    for (auto& [name, field] : klassPtr->VTable->Fields)
                    {
                        s_ScriptData->EntityClasses[klass->ClassName]->m_Fields[name] = ScriptField{CapyTypeStringToScriptFieldType(field->FieldTypeString), name, field.get()};
                    }

                }
            }

            std::vector<std::string> coreLib = capy_get_core_libraries_from_domain("CatAppDomain");


            ScriptGlue::RegisterFunctions();
            ScriptGlue::RegisterComponents();

            CE_API_WARN("{}", capy_dump_domain("CatAppDomain"));
        });

        s_ScriptData->SourceFileReloadPending = false;
        LoadFileWatcher(Project::GetAssetDirectory());
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

			// Copy field value
			if (s_ScriptData->EntityScriptFields.find(entityID) != s_ScriptData->EntityScriptFields.end())
			{
				const ScriptFieldMap& scriptFieldMap = s_ScriptData->EntityScriptFields.at(entityID);

				for (const auto& [name, fieldInstance] : scriptFieldMap)
				{
					instance->SetFieldDataInternal(name, (void*)fieldInstance.m_Data);
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
		
		
        UUID entityID = entity.GetUUID();
        return s_ScriptData->EntityScriptFields[entityID];
		
	}
}
