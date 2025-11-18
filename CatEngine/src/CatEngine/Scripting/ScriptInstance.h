#pragma once

#include "CatScriptCore.h"
#include "CatEngine/Scene/Entity.h"
#include <unordered_map>

namespace CatEngine
{
    class ScriptClass;
    class ScriptInstance
	{
	public:
		ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity);
        ~ScriptInstance();

		void InvokeUpdateMethod(float ts);
		void InvokeStartMethod();
        void InvokeDeleteScript();

        void InvokeOnCollisionEnter(const UUID& other);
        void InvokeOnCollisionExit(const UUID& other);

		Ref<ScriptClass> GetScriptClass() { return m_ScriptClass; }

		template<typename T>
		T GetFieldData(const std::string& name)
		{
			static_assert(sizeof(T) <= 128, "Type to large!");
			bool success = GetFieldDataInternal(name, &s_FieldValueBuffer);
			if (success)
                return *(T*)s_FieldValueBuffer;

            return T();
		}

		template<typename T>
		void SetFieldData(const std::string& name, T value)
		{
			static_assert(sizeof(T) <= 128, "Type to large!");
			SetFieldDataInternal(name, &value);
		}
		//CatScriptObject* GetManagedObject() { return m_Instance; }

	private:
		bool GetFieldDataInternal(const std::string& name, void* buffer);
		void SetFieldDataInternal(const std::string& name, void* value);
	private:
		Ref<ScriptClass> m_ScriptClass;

        CapyMethod* m_StartMethod;
        CapyMethod* m_UpdateMethod;
        CapyMethod* m_CollisionEnterMethod;
        CapyMethod* m_CollisionExitMethod;

		void* m_Instance = nullptr;

        std::unordered_map<std::string, std::vector<uint8_t>> m_DefaultFieldDatas;

		static inline char s_FieldValueBuffer[8];

		friend class ScriptEngine;
		friend struct ScriptFieldInstance;
	};
}
