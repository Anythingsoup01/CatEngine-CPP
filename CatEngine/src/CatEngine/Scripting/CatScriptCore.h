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
        mutable UUID ID;
	};

	struct ScriptFieldInstance
	{
        std::string Name;
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
			memcpy(m_Data, &value, TypeToSize(Field.Type));
		}

	private:

		char m_Data[128];

		friend class ScriptEngine;
		friend class ScriptInstance;
	};

    struct ScriptFieldSerializedValue
    {
        std::string FieldName;
        ScriptFieldType FieldType;

        union
        {
            float       f;
            double      d;
            char        c;
            bool        b;
            int8_t      i8;
            int16_t     i16;
            int32_t     i32;
            int64_t     i64;
            uint8_t     ui8;
            uint16_t    ui16;
            uint32_t    ui32;
            uint64_t    ui64;
        };

        std::string s;
        glm::vec2 v2;
        glm::vec3 v3;
        glm::vec4 v4;

        UUID id;

        template<typename T>
        void SetValue(const T& val)
        {
        }

        template<float> void SetValue(const float& val)          { f = val; }
        template<double> void SetValue(const double& val)         { d = val; }
        template<char> void SetValue(const char& val)           { c = val; }
        template<bool> void SetValue(const bool& val)           { b = val; }
        template<int8_t> void SetValue(const int8_t& val)         { i8 = val; }
        template<int16_t> void SetValue(const int16_t& val)        { i16 = val; }
        template<int32_t> void SetValue(const int32_t& val)        { i32 = val; }
        template<int64_t> void SetValue(const int64_t& val)        { i64 = val; }
        template<uint8_t> void SetValue(const uint8_t& val)        { ui8 = val; }
        template<uint16_t> void SetValue(const uint16_t& val)       { ui16 = val; }
        template<uint32_t> void SetValue(const uint32_t& val)       { ui32 = val; }
        template<glm::vec2> void SetValue(const glm::vec2& val)      { v2 = val; }
        template<glm::vec3> void SetValue(const glm::vec3& val)      { v3 = val; }
        template<glm::vec4> void SetValue(const glm::vec4& val)      { v4 = val; }

        // THIS IS USED IN PLACE OF UUID, TEMPLATES DON'T LIKE IT FOR SOME REASON
        // TODO: FIX THIS
        template<uint64_t> void SetValue(const uint64_t& val)           { id = val; }

        ScriptFieldSerializedValue() = default;
        ScriptFieldSerializedValue(const std::string& name, const ScriptFieldType& type, const float& val) :         FieldName(name), FieldType(type), f(val) {}
        ScriptFieldSerializedValue(const std::string& name, const ScriptFieldType& type, const double& val) :        FieldName(name), FieldType(type), d(val) {}
        ScriptFieldSerializedValue(const std::string& name, const ScriptFieldType& type, const char& val) :          FieldName(name), FieldType(type), c(val) {}
        ScriptFieldSerializedValue(const std::string& name, const ScriptFieldType& type, const bool& val) :          FieldName(name), FieldType(type), b(val) {}
        ScriptFieldSerializedValue(const std::string& name, const ScriptFieldType& type, const int8_t& val) :        FieldName(name), FieldType(type), i8(val) {}
        ScriptFieldSerializedValue(const std::string& name, const ScriptFieldType& type, const int16_t& val) :       FieldName(name), FieldType(type), i16(val) {}
        ScriptFieldSerializedValue(const std::string& name, const ScriptFieldType& type, const int32_t& val) :       FieldName(name), FieldType(type), i32(val) {}
        ScriptFieldSerializedValue(const std::string& name, const ScriptFieldType& type, const int64_t& val) :       FieldName(name), FieldType(type), i64(val) {}
        ScriptFieldSerializedValue(const std::string& name, const ScriptFieldType& type, const uint8_t& val) :       FieldName(name), FieldType(type), ui8(val) {}
        ScriptFieldSerializedValue(const std::string& name, const ScriptFieldType& type, const uint16_t& val) :      FieldName(name), FieldType(type), ui16(val) {}
        ScriptFieldSerializedValue(const std::string& name, const ScriptFieldType& type, const uint32_t& val) :      FieldName(name), FieldType(type), ui32(val) {}
        ScriptFieldSerializedValue(const std::string& name, const ScriptFieldType& type, const uint64_t& val) :      FieldName(name), FieldType(type), ui64(val) {}
        ScriptFieldSerializedValue(const std::string& name, const ScriptFieldType& type, const std::string& val) :   FieldName(name), FieldType(type), s(val) {}
        ScriptFieldSerializedValue(const std::string& name, const ScriptFieldType& type, const glm::vec2& val) :     FieldName(name), FieldType(type), v2(val) {}
        ScriptFieldSerializedValue(const std::string& name, const ScriptFieldType& type, const glm::vec3& val) :     FieldName(name), FieldType(type), v3(val) {}
        ScriptFieldSerializedValue(const std::string& name, const ScriptFieldType& type, const glm::vec4& val) :     FieldName(name), FieldType(type), v4(val) {}
        ScriptFieldSerializedValue(const std::string& name, const ScriptFieldType& type, const UUID& val) :          FieldName(name), FieldType(type), id(val) {}
    };

    struct ScriptFieldMetaData
    {
        ScriptFieldSerializedValue FieldStorage;
    };

}
