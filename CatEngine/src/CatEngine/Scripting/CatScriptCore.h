#pragma once


#include <Capybara/Capybara.h>
#include <glm/glm.hpp>

#include "CatEngine/Core/UUID.h"

namespace CatEngine
{

    enum class ScriptFieldType
	{
		None = 0,
		Float, Double,
		SByte, Char, Int16, Int32, Int64, Boolean,
		UInt16, UInt32, UInt64,
		String,
		Vector2, Vector3, Vector4, Texture2D,
		TransformComponent, Rigidbody2DComponent, SpriteRenderer,
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
        {"Texture2D", ScriptFieldType::Texture2D},
		{"Transform", ScriptFieldType::TransformComponent},
		{"Rigidbody2D", ScriptFieldType::Rigidbody2DComponent},
        {"SpriteRenderer", ScriptFieldType::SpriteRenderer},
        {"Layer", ScriptFieldType::None},
        {"Tag", ScriptFieldType::None},
	};

    static std::string ScriptFieldTypeToString(const ScriptFieldType& type)
	{
        switch (type)
        {
            case ScriptFieldType::Float: return "Float";
            case ScriptFieldType::Double: return "Double";
            case ScriptFieldType::Char: return "Char";
            case ScriptFieldType::Int16: return "Int16";
            case ScriptFieldType::Int32: return "Int32";
            case ScriptFieldType::Int64: return "Int64";
            case ScriptFieldType::Boolean: return "Boolean";
            case ScriptFieldType::UInt16: return "UInt16";
            case ScriptFieldType::UInt32: return "UInt32";
            case ScriptFieldType::UInt64: return "UInt64";
            case ScriptFieldType::String: return "String";
            case ScriptFieldType::Vector2: return "Vector2";
            case ScriptFieldType::Vector3: return "Vector3";
            case ScriptFieldType::Vector4: return "Vector4";
            case ScriptFieldType::Texture2D: return "Texture2D";
            case ScriptFieldType::TransformComponent: return "Transform";
            case ScriptFieldType::Rigidbody2DComponent: return "Rigidbody2D";
            case ScriptFieldType::SpriteRenderer: return "SpriteRenderer";
            default: return "null";
        }
	}

    static ScriptFieldType StringToScriptFieldType(const std::string& typeStr)
	{
            if (typeStr == "Float")             return ScriptFieldType::Float;
            if (typeStr == "Double")            return ScriptFieldType::Double;
            if (typeStr == "Char")              return ScriptFieldType::Char;
            if (typeStr == "Boolean")           return ScriptFieldType::Boolean;
            if (typeStr == "Int16")             return ScriptFieldType::Int16;
            if (typeStr == "Int32")             return ScriptFieldType::Int32;
            if (typeStr == "Int64")             return ScriptFieldType::Int64;
            if (typeStr == "UInt16")            return ScriptFieldType::UInt16;
            if (typeStr == "UInt32")            return ScriptFieldType::UInt32;
            if (typeStr == "UInt64")            return ScriptFieldType::UInt64;
            if (typeStr == "String")            return ScriptFieldType::String;
            if (typeStr == "Vector2")           return ScriptFieldType::Vector2;
            if (typeStr == "Vector3")           return ScriptFieldType::Vector3;
            if (typeStr == "Vector4")           return ScriptFieldType::Vector4;
            if (typeStr == "Texture2D")         return ScriptFieldType::Texture2D;
            if (typeStr == "Transform")         return ScriptFieldType::TransformComponent;
            if (typeStr == "Rigidbody2D")       return ScriptFieldType::Rigidbody2DComponent;
            if (typeStr == "SpriteRenderer")    return ScriptFieldType::SpriteRenderer;
            return ScriptFieldType::None;
	}

    static ScriptFieldType CapyTypeStringToScriptFieldType(const std::string& capyTypeStr)
    {
        auto it = s_ScriptFieldTypeMap.find(capyTypeStr);

        if (it != s_ScriptFieldTypeMap.end())
            return it->second;

        CE_API_ERROR("Not supported type {}", capyTypeStr);
        return ScriptFieldType::None;
    }

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
            case ScriptFieldType::Texture2D: return sizeof(UUID);
            case ScriptFieldType::TransformComponent: return sizeof(UUID);
            case ScriptFieldType::Rigidbody2DComponent: return sizeof(UUID);
            case ScriptFieldType::SpriteRenderer: return sizeof(UUID);
            default : return 0;
        }
    }
    
    struct ScriptField
	{
		std::string Name;
		ScriptFieldType Type;
        mutable std::string ComponentName;
	};

	struct ScriptFieldInstance
	{
        std::string Name;
		ScriptField Field;
		
		ScriptFieldInstance()
		{
			m_Data.Allocate(128);
		}

		template<typename T>
		T GetValue()
		{
			static_assert(sizeof(T) <= 128, "Type to large!");
		    return m_Data.Read<T>();
        }

		template<typename T>
		T GetValue() const
		{
			static_assert(sizeof(T) <= 128, "Type to large!");
		    return m_Data.Read<T>();
        }
		
        template<typename T>
		void SetValue(T value)
		{
			static_assert(sizeof(T) <= 128, "Type to large!");
			m_Data.Write(&value, TypeToSize(Field.Type));
		}

	private:

		Buffer m_Data;

		friend class ScriptEngine;
		friend class ScriptInstance;
	};
}
