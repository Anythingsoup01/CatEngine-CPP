#pragma once

#include "InternalObjectData.h"
#include "CatScriptCore/InternalCalls/InternalCalls.h"

namespace CatRuntime
{
    struct Layer
    {
    public:
        uint64_t id;

        operator std::string() const 
        {
            if (id <= 0) return std::string();
            return Object_GetLayer(id);
        }

        bool operator==(const std::string& other) const 
        {
            if (id <= 0) return false;
            std::string thisStr = Object_GetLayer(id);

            if (thisStr == other)
                return true;

            return false;
        }

        bool operator==(const char* other) const 
        {
            if (id <= 0) return false;
            std::string thisStr = Object_GetLayer(id);
            std::string otherStr(other);
            if (thisStr == otherStr)
                return true;

            return false;
        }

        Layer& operator=(const std::string& other)
        {
            if (id <= 0) return *this;
            Object_SetLayer(id, other.c_str());
            return *this;
        }
    };
}

std::ostream& operator<<(std::ostream& os, const CatRuntime::Layer& proxy);


