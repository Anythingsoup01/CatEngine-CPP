#pragma once

#include "CatScriptCore/InternalCalls/InternalCalls.h"
#include "InternalObjectData.h"

namespace CatRuntime
{
	struct Tag
	{
    public:
        uint64_t id;

        struct TagProxy
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

        TagProxy layer() { return TagProxy{id}; }
    };
}

std::ostream& operator<<(std::ostream& os, const CatRuntime::Tag& proxy);
