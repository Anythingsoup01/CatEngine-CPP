#pragma once

#include "IScriptObject.h"

namespace CatEngine
{
    typedef IScriptObject CatScriptObject;
    typedef void CatScriptClass;
    typedef void CatScriptField;
    typedef void CatScriptMethod;

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
}
