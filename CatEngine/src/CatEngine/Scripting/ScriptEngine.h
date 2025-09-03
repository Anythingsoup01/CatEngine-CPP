#include <filesystem>

#include "CatEngine/Core/UUID.h"
#include "IScriptObject.h"

#include "SourceFileCompiler.h"


typedef CatEngine::IScriptObject CatScriptObject;
typedef void CatScriptClass;
typedef void CatScriptField;
typedef void CatScriptMethod;

namespace CatEngine
{
    class Scene;
    class Entity;

    enum class ScriptFieldType
	{
		None = 0,
		Float, Double,
		SByte, Char, Int16, Int32, Int64, Boolean,
		Byte, UInt16, UInt32, UInt64,
		String,
		Vector2, Vector3, Vector4,
		TransformComponent, Rigidbody2DComponent,
	};

	struct ScriptField
	{
		ScriptFieldType Type;
		std::string Name;
		CatScriptField* ClassField;
	};

	struct ScriptFieldInstance
	{
		ScriptField Field;
		ScriptFieldInstance()
		{
			memset(m_Data, 0, sizeof(m_Data));
		}

		template<typename T>
		T GetValue()
		{
			static_assert(sizeof(T) <= 128, "Type to large!");
			return *(T*)m_Data;
		}

		template<typename T>
		void SetValue(T value)
		{
			static_assert(sizeof(T) <= 128, "Type to large!");
			memcpy(m_Data, &value, sizeof(T));
		}


	private:

		char m_Data[128];

		friend class ScriptEngine;
		friend class ScriptInstance;
	};
    
    using ScriptFieldMap = std::unordered_map<std::string, ScriptFieldInstance>;
    
    class ScriptClass
    {
    public:
        ScriptClass() = default;
        ScriptClass(const std::filesystem::path& path);

        void SetFieldsFromFile(const std::filesystem::path& filePath);

        CatScriptObject* Instantiate() { return m_CreateScript(); }
        CatScriptMethod* GetMethod(const std::string& methodName);

        void DeleteScript(IScriptObject* scriptObject) { delete scriptObject; }

        const std::unordered_map<std::string, ScriptField>& GetFields() const { return m_Fields; }
    private:
        std::filesystem::path m_Path;
        
        CatScriptClass* m_Instance;

        create_t* m_CreateScript;
        destroy_t* m_DestroyScript;

        std::unordered_map<std::string, ScriptField> m_Fields;

    };
    
    class ScriptInstance
	{
	public:
		ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity);

        void SetEntityID(UUID entityID);

		void InvokeUpdateMethod(float ts);
		void InvokeStartMethod();

		Ref<ScriptClass> GetScriptClass() { return m_ScriptClass; }

		template<typename T>
		T GetFieldData(const std::string& name)
		{
			static_assert(sizeof(T) <= 128, "Type to large!");
			bool success = GetFieldDataInternal(name, &s_FieldValueBuffer);
			if (!success)
				return T();
			return *(T*)s_FieldValueBuffer;
		}

		template<typename T>
		void SetFieldData(const std::string& name, T value)
		{
			static_assert(sizeof(T) <= 128, "Type to large!");
			SetFieldDataInternal(name, value);
		}
		CatScriptObject* GetManagedObject() { return m_Instance; }

	private:
		bool GetFieldDataInternal(const std::string& name, void* buffer);
		void SetFieldDataInternal(const std::string& name, void* value);
	private:
		Ref<ScriptClass> m_ScriptClass;

		CatScriptObject* m_Instance = nullptr;

		static inline char s_FieldValueBuffer[8];

		friend class ScriptEngine;
		friend struct ScriptFieldInstance;
        friend class IScriptObject;
	};
    
    class ScriptEngine
    {
	public:
		static void Init();
		static void Shutdown();

		static bool LoadFileWatcher(const std::filesystem::path& filePath);

		static void SetSceneContext(Ref<Scene> scene);
		static void OnRuntimeStop();

		static void ReloadBinaries();

		static bool ScriptClassExists(const std::string& fullClassName);
		static void OnStartEntity(Entity e);
		static void OnUpdateEntity(Entity e, float ts);

		static Ref<Scene> GetSceneContext();
		static Ref<ScriptInstance> GetEntityScriptInstance(UUID entityID);

		static CatScriptObject* GetManagedInstance(UUID uuid);

		static std::unordered_map<std::string, Ref<ScriptClass>>& GetScriptClasses();
		static Ref<ScriptClass> GetScriptClass(const std::string& name);
		static ScriptFieldMap& GetScriptFieldMap(Entity entity);

	private:

        static inline SourceFileCompiler m_Compiler = SourceFileCompiler();

		friend class ScriptClass;
	};
}
