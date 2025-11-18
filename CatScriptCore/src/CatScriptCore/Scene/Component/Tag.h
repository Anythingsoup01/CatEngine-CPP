#pragma once

#include "CatScriptCore/InternalCalls/InternalCalls.h"
#include "InternalObjectData.h"

namespace CatRuntime
{
	class Tag : InternalObjectData
	{
    public:
        Tag() = default;
        Tag(uint64_t entityID) : InternalObjectData(entityID) {}

        class TagProxy
        {
        public:
            uint64_t id;

            operator std::string() const 
            {
                if (id <= 0) return std::string();
                return Object_GetTag(id);
            }

            TagProxy& operator=(const std::string& other)
            {
                if (id <= 0) return *this;
                Object_SetTag(id, other.c_str());
                return *this;
            }
        };

        TagProxy layer() { return TagProxy{m_InstanceID}; }
	};
}
