#include "cepch.h"
#include "ScriptEngine.h"

#include "FileWatch.hpp"

#include "CatEngine/Scene/Scene.h"
#include "CatEngine/Scene/Entity.h"


#include <glm/glm.hpp>
#include <dlfcn.h>

#include "CatEngine/Core/Application.h"

#include "CatEngine/Scene/Components/Components.h"

namespace CatEngine
{
    static std::unordered_map<std::string, ScriptFieldType> s_ScriptFieldTypeMap =
	{
		{"float", ScriptFieldType::Float},
		{"double", ScriptFieldType::Double},
		{"char",ScriptFieldType::Char},
		{"int16_t", ScriptFieldType::Int16},
		{"int32_t", ScriptFieldType::Int32},
		{"int", ScriptFieldType::Int32},
		{"int64_t", ScriptFieldType::Int64},
		{"bool", ScriptFieldType::Boolean},
		{"uint16_t", ScriptFieldType::UInt16},
		{"uint32_t", ScriptFieldType::UInt32},
		{"unsigned int", ScriptFieldType::UInt32},
		{"uint64_t", ScriptFieldType::UInt64},
		{"std::string", ScriptFieldType::String},
		{"glm::vec2", ScriptFieldType::Vector2},
		{"glm::vec3", ScriptFieldType::Vector3},
		{"glm::vec4", ScriptFieldType::Vector4},
		{"TransformComponent", ScriptFieldType::TransformComponent},
		{"Rigidbody2DComponent", ScriptFieldType::Rigidbody2DComponent},

	};
		
    ScriptFieldType StringToScriptFieldType(const std::string& type)
    {
        auto it = s_ScriptFieldTypeMap.find(type);

        if (it != s_ScriptFieldTypeMap.end())
            return it->second;

        CE_API_ERROR("Not supported type {}", type);
        return ScriptFieldType::None;
    }

    void* GetVariableSymbol(void* handle, ScriptFieldType type, const std::string& symbolName)
    {
        void* out = nullptr;
        switch (type)
        {
            case ScriptFieldType::Float: out = (float*)dlsym(handle, symbolName.c_str()); break;
            case ScriptFieldType::Double: out = (double*)dlsym(handle, symbolName.c_str()); break;
            case ScriptFieldType::Char: out = (char**)dlsym(handle, symbolName.c_str()); break;
            case ScriptFieldType::Int16: out = (int16_t*)dlsym(handle, symbolName.c_str()); break;
            case ScriptFieldType::Int32: out = (int32_t*)dlsym(handle, symbolName.c_str()); break;
            case ScriptFieldType::Int64: out = (int64_t*)dlsym(handle, symbolName.c_str()); break;
            case ScriptFieldType::Boolean: out = (bool*)dlsym(handle, symbolName.c_str()); break;
            case ScriptFieldType::UInt16: out = (uint16_t*)dlsym(handle, symbolName.c_str()); break;
            case ScriptFieldType::UInt32: out = (uint32_t*)dlsym(handle, symbolName.c_str()); break;
            case ScriptFieldType::UInt64: out = (uint64_t*)dlsym(handle, symbolName.c_str()); break;
            case ScriptFieldType::String: out = (std::string*)dlsym(handle, symbolName.c_str()); break;
            case ScriptFieldType::Vector2: out = (glm::vec2*)dlsym(handle, symbolName.c_str()); break;
            case ScriptFieldType::Vector3: out = (glm::vec3*)dlsym(handle, symbolName.c_str()); break;
            case ScriptFieldType::Vector4: out = (glm::vec4*)dlsym(handle, symbolName.c_str()); break;
            case ScriptFieldType::TransformComponent: out = (TransformComponent*)dlsym(handle, symbolName.c_str()); break;
            case ScriptFieldType::Rigidbody2DComponent: out = (Rigidbody2DComponent*)dlsym(handle, symbolName.c_str()); break;
            default: break;
        }

        return out;

    }

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
    /////////////////////////////////////////////////////////////////////
    ///    ScriptClass    ///////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////

    ScriptClass::ScriptClass(const std::filesystem::path& path)
        : m_Path(path)
    {
        m_Instance = dlopen(path.c_str(), RTLD_LAZY);
        CE_API_ASSERT(m_Instance, dlerror());

        m_CreateScript = (create_t*)dlsym(m_Instance, "create");
        m_DestroyScript = (destroy_t*)dlsym(m_Instance, "destroy");
    }

    void ScriptClass::SetFieldsFromFile(const std::filesystem::path& filePath)
    {
        std::unordered_map<std::string, std::string> variables;
    
        std::ifstream in(filePath);
        if (!in.is_open())
        {
            fprintf(stderr, "Failed to open file!\n");
            return;
        }

        std::string line;

        while (std::getline(in, line))
        {
            if (strncmp(line.c_str(), "extern \"C\" CatEngine::IScriptObject* create()", 33) == 0)
                break;

            else if (strncmp(line.c_str(), "extern \"C\"", 10) == 0)
            {
                size_t linePos = 11; // One more than ^
                size_t space = line.find_first_of(" ", linePos);
                std::string variableType = line.substr(linePos, space - linePos);
                linePos = space + 1;

                if (strncmp(variableType.c_str(), "const", 5) == 0)
                {
                    space = line.find_first_of(" ", linePos);
                    std::string extra = line.substr(linePos, space - linePos);
                    variableType.append(" ").append(extra);
                    linePos = space + 1;
                }


                space = line.find_first_of(" ", linePos);
                std::string variableName = line.substr(linePos, space - linePos);

                variables.emplace(std::pair<std::string, std::string>(variableName, variableType));
            }
        }

        for (auto& [name, type] : variables)
        {
            ScriptField sf;
            sf.Type = StringToScriptFieldType(type);
            sf.Name = name;
            sf.ClassField = GetVariableSymbol(m_Instance, sf.Type, name);

            m_Fields.emplace(std::pair<std::string, ScriptField>(name, sf));
        }

    }

    CatScriptMethod* ScriptClass::GetMethod(const std::string& methodName)
    {
        CatScriptMethod* method = (CatScriptMethod*)dlsym(m_Instance, methodName.c_str());
        CE_API_ASSERT(method, "Could Not Load: {}", dlerror());

        return method;
    }
    
    ///////////////////////////////////////////////////////
	// SCRIPT INSTANCE ////////////////////////////////////
	///////////////////////////////////////////////////////

	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity)
		: m_ScriptClass(scriptClass)
	{ 
		// Put specific events here - i.e. Start, Update, Awake, Collision, Possibly Triggers
		m_Instance = scriptClass->Instantiate();

	}

    void ScriptInstance::SetEntityID(UUID entityID)
    {
        m_Instance->SetUUID(entityID);
    }

	void ScriptInstance::InvokeUpdateMethod(float ts)
	{
        m_Instance->Update(ts);
	}

	void ScriptInstance::InvokeStartMethod()
	{
        m_Instance->Start();
    }
	bool ScriptInstance::GetFieldDataInternal(const std::string& name, void* buffer)
    {
        const auto& fields = m_ScriptClass->GetFields();
        auto it = fields.find(name);

        if (it == fields.end())
            return false;

        const ScriptField& field = it->second;
        memcpy(buffer, field.ClassField, sizeof(field.ClassField));
        return true;
    }

	void ScriptInstance::SetFieldDataInternal(const std::string& name, void* value)
    {
        const auto& fields = m_ScriptClass->GetFields();
        auto it = fields.find(name);

        if (it == fields.end())
            return;

        const ScriptField& field = it->second;
        switch (field.Type) 
        {
            case ScriptFieldType::Float: *(float*)field.ClassField = *(float*)value; break;
            case ScriptFieldType::Double: *(double*)field.ClassField = *(double*)value; break;
            case ScriptFieldType::Char: *(char*)field.ClassField = *(char*)value; break;
            case ScriptFieldType::Int16: *(int16_t*)field.ClassField = *(int16_t*)value; break;
            case ScriptFieldType::Int32: *(int32_t*)field.ClassField = *(int32_t*)value; break;
            case ScriptFieldType::Int64: *(int64_t*)field.ClassField = *(int64_t*)value; break;
            case ScriptFieldType::Boolean: *(bool*)field.ClassField = *(bool*)value; break;
            case ScriptFieldType::UInt16: *(uint16_t*)field.ClassField = *(uint16_t*)value; break;
            case ScriptFieldType::UInt32: *(uint32_t*)field.ClassField = *(uint32_t*)value; break;
            case ScriptFieldType::UInt64: *(uint64_t*)field.ClassField = *(uint64_t*)value; break;
            case ScriptFieldType::String: *(std::string*)field.ClassField = *(std::string*)value; break;
            case ScriptFieldType::Vector2: *(glm::vec2*)field.ClassField = *(glm::vec2*)value; break;
            case ScriptFieldType::Vector3: *(glm::vec3*)field.ClassField = *(glm::vec3*)value; break;
            case ScriptFieldType::Vector4: *(glm::vec4*)field.ClassField = *(glm::vec4*)value; break;
            case ScriptFieldType::TransformComponent: *(TransformComponent*)field.ClassField = *(TransformComponent*)value; break;
            case ScriptFieldType::Rigidbody2DComponent: *(Rigidbody2DComponent*)field.ClassField = *(Rigidbody2DComponent*)value; break;
        }
    }

	///////////////////////////////////////////////////////
	// SCRIPT ENGINE //////////////////////////////////////
	///////////////////////////////////////////////////////
    void ScriptEngine::Init()
	{
		s_ScriptData = new ScriptEngineData();
        SourceFileCompiler::Init();
        SourceFileCompiler::AddDirectory("SampleProject/Assets/");
        ReloadBinaries();

	}
	void ScriptEngine::Shutdown()
	{
        s_ScriptData->EntityScriptFields.clear();
        s_ScriptData->SourceFileWatchers.clear();
        s_ScriptData->EntityClasses.clear();
        s_ScriptFieldTypeMap.clear();
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
					Application::Get().SubmitToMainThread([]()
					{
                        for (auto& fileWatcher : s_ScriptData->SourceFileWatchers)
                            fileWatcher.reset();
                        ScriptEngine::LoadFileWatcher("SampleProject/Assets/");
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
        LoadFileWatcher("SampleProject/Assets/");
	}

	void ScriptEngine::OnRuntimeStop()
	{
		s_ScriptData->SceneContext = nullptr;
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
        s_ScriptData->EntityClasses.clear();
        m_Compiler.CopyAndPrepareFiles();
        m_Compiler.CompileFiles();

        std::unordered_map<std::filesystem::path, FileDescription> intermediates = m_Compiler.GetIntermediateFiles();

        Application::Get().SubmitToMainThread([intermediates](){
            for (auto& [path, fd] : intermediates)
            {
                s_ScriptData->EntityClasses[fd.Name] = CreateRef<ScriptClass>(fd.CompilePath);
                s_ScriptData->EntityClasses[fd.Name]->SetFieldsFromFile(fd.Path);
            }
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
