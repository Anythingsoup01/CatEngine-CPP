#pragma once


#include <capybara/capybara.h>
#include <glm/glm.hpp>

#include "CatEngine/Core/UUID.h"

namespace CatEngine
{
    enum class DataType
	{
        Short,
        UShort,
        Int,
        UInt,
        Long,
        ULong,

        Float,
        Double,

        Bool,
		
        Vector2,
        Vector3,
        Vector4,

        Entity,

        Texture2D,
	};

    inline uint64_t TypeToSize(const DataType& type)
    {
        switch (type)
        {
            case DataType::Short: return sizeof(int16_t);
            case DataType::UShort: return sizeof(uint16_t);
            case DataType::Int: return sizeof(int32_t);
            case DataType::UInt: return sizeof(uint32_t);
            case DataType::Long: return sizeof(int64_t);
            case DataType::ULong: return sizeof(uint64_t);
            case DataType::Float: return sizeof(float);
            case DataType::Double: return sizeof(bool);
            case DataType::Bool: return sizeof(bool);
            case DataType::Vector2: return sizeof(float) * 2;
            case DataType::Vector3: return sizeof(float) * 3;
            case DataType::Vector4: return sizeof(float) * 4;
            case DataType::Entity: return sizeof(UUID);
            case DataType::Texture2D: return sizeof(UUID);
        }
    }

    

    
    struct ScriptField
	{
		DataType Type;
        std::string Name;
        CapyField* Field;
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

    using ScriptFieldMap = std::unordered_map<UUID, ScriptFieldInstance>;
}
