#pragma once

#include "CatScriptCore.h"

namespace CatEngine
{
    class ScriptClass;
    class ScriptInstance
	{
	public:
		ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity);

        void SetEntityID(UUID entityID);

		void InvokeUpdateMethod(float ts);
		void InvokeStartMethod();
        void InvokeDeleteScript();

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
}
