#pragma once

#include "CatEngine/Scene/Components/3D/TransformComponent.h"
#include "CatEngine/Scene/Components/Physics/Rigidbody2D.h"
#include "glm/fwd.hpp"
#include <cstdint>

#include <Capybara/Capybara.h>

namespace CatEngine
{

    enum class ScriptFieldType
	{
		None = 0,
		Float, Double,
		SByte, Char, Int16, Int32, Int64, Boolean,
		UInt16, UInt32, UInt64,
		String,
		Vector2, Vector3, Vector4,
		TransformComponent, Rigidbody2DComponent,
	};

    static std::unordered_map<std::string, ScriptFieldType> s_ScriptFieldTypeMap =
	{
		{"float", ScriptFieldType::Float},
		{"double", ScriptFieldType::Double},
		{"null",ScriptFieldType::SByte},
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
		{"Vector2", ScriptFieldType::Vector2},
		{"Vector3", ScriptFieldType::Vector3},
		{"Vector4", ScriptFieldType::Vector4},
		{"Transform", ScriptFieldType::TransformComponent},
		{"Rigidbody2D", ScriptFieldType::Rigidbody2DComponent},

	};

    static ScriptFieldType CapyTypeStringToScriptFieldType(const std::string& capyTypeStr)
    {
        auto it = s_ScriptFieldTypeMap.find(capyTypeStr);

        if (it != s_ScriptFieldTypeMap.end())
            return it->second;

        CE_API_ERROR("Not supported type {}", capyTypeStr);
        return ScriptFieldType::None;
    }

    struct ScriptField
	{
		ScriptFieldType Type;
		std::string Name;
		CapyField* ClassField;
	};

	struct ScriptFieldInstance
	{
		ScriptField Field;
		
		ScriptFieldInstance()
		{
			memset(m_Data, 0, sizeof(m_Data));
		}

		template<typename T>
		void GetValue(void* value)
		{
			static_assert(sizeof(T) <= 128, "Type to large!");
			memcpy(value, m_Data, sizeof(T));
		}

		template<typename T>
		void SetValue(T* value)
		{
			static_assert(sizeof(T) <= 128, "Type to large!");
			memcpy(m_Data, value, sizeof(T));
		}

	private:

		char m_Data[128];

		friend class ScriptEngine;
		friend class ScriptInstance;
	};

    static size_t TypeToSize(const ScriptFieldType& type)
    {
        switch (type) 
        {
            case ScriptFieldType::Float: return sizeof(float);
            case ScriptFieldType::Double: return sizeof(double);
            case ScriptFieldType::SByte: return sizeof(int8_t);
            case ScriptFieldType::Char: return sizeof(char);
            case ScriptFieldType::Int16: return sizeof(int16_t);
            case ScriptFieldType::Int32: return sizeof(int32_t);
            case ScriptFieldType::Int64: return sizeof(int64_t);
            case ScriptFieldType::Boolean: return sizeof(bool);
            case ScriptFieldType::UInt16: return sizeof(uint16_t);
            case ScriptFieldType::UInt32: return sizeof(uint32_t);
            case ScriptFieldType::UInt64: return sizeof(uint64_t);
            case ScriptFieldType::String: return sizeof(std::string);
            case ScriptFieldType::Vector2: return sizeof(glm::vec2);
            case ScriptFieldType::Vector3: return sizeof(glm::vec3);
            case ScriptFieldType::Vector4: return sizeof(glm::vec4);
            case ScriptFieldType::TransformComponent: return sizeof(TransformComponent);
            case ScriptFieldType::Rigidbody2DComponent: return sizeof(Rigidbody2DComponent);
            default : return 0;
        }
    }
}
