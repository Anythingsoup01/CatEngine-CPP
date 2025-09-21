#include "cepch.h"
#include "ScriptEngine.h"

#include "FileWatch.hpp"

#include "CatEngine/Scene/Scene.h"
#include "CatEngine/Scene/Entity.h"


#include <glm/glm.hpp>


#include "CatEngine/Core/Application.h"

#include "CatEngine/Scene/Components/Components.h"

#include "CatEngine/Project/Project.h"

static bool s_ReloadFileWatcher = false;

namespace CatEngine
{
	struct ScriptEngineData
	{
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
		s_ScriptData = new ScriptEngineData();
	}
	void ScriptEngine::Shutdown()
	{
        s_ScriptData->EntityScriptFields.clear();
        s_ScriptData->SourceFileWatchers.clear();
        s_ScriptData->EntityClasses.clear();
		delete s_ScriptData;
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

		return true;
	}

	void ScriptEngine::SetSceneContext(Ref<Scene> scene)
	{
		s_ScriptData->SceneContext = scene;
	}

	void ScriptEngine::OnRuntimeStop()
	{
		s_ScriptData->SceneContext = nullptr;
        std::vector<UUID> scriptUUIDs;
        for (auto& [uuid, instance] : s_ScriptData->EntityInstances)
        {
            instance->InvokeDeleteScript();
            scriptUUIDs.push_back(uuid);
        }

        for (auto& uuid : scriptUUIDs)
        {
            //s_ScriptData->EntityInstances[uuid]->ResetFieldData();
        }


		s_ScriptData->EntityInstances.clear();
		
		if (s_ScriptData->SourceFileReloadPending || !s_ScriptData->MainThreadQueue.empty())
			Application::Get().SubmitToMainThread(s_ScriptData->MainThreadQueue.at(0));
	}

	void ScriptEngine::ReloadBinaries()
	{
#ifdef CE_RELEASE
		//CE_CLI_TRACE(s_ScriptData->ReloadTimer.ElapsedMillis());
#endif
		//CE_API_TRACE(s_ScriptData->ReloadTimer.ElapsedMillis());

        Application::Get().SubmitToMainThread([](){
            
            auto& compiledFiles = SourceFileCompiler::GetCompiledFiles();
            
            for (auto& fd : compiledFiles)
            {
                auto it = s_ScriptData->EntityClasses.find(fd.Name);
                if (it != s_ScriptData->EntityClasses.end())
                {
                    s_ScriptData->EntityClasses.erase(it);
                }
                s_ScriptData->EntityClasses[fd.Name] = CreateRef<ScriptClass>(fd.SharedObjectPath);
                s_ScriptData->EntityClasses[fd.Name]->SetFieldsFromFile(fd.SourceFilePath);
            }
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
            instance->SetEntityID(entityID);
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
    
	CatScriptObject* ScriptEngine::GetManagedInstance(UUID uuid)
	{
		if(s_ScriptData->EntityInstances.find(uuid) != s_ScriptData->EntityInstances.end())
			return s_ScriptData->EntityInstances.at(uuid)->GetManagedObject();

		return nullptr;
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
