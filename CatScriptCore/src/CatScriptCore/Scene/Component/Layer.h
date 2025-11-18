#pragma once

#include "InternalObjectData.h"
#include "CatScriptCore/InternalCalls/InternalCalls.h"

namespace CatRuntime
{
	class Layer : InternalObjectData
	{
    public:
        Layer() = default;
        Layer(uint64_t entityID) : InternalObjectData(entityID) {}

        class LayerProxy
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



            LayerProxy& operator=(const std::string& other)
            {
                if (id <= 0) return *this;
                Object_SetLayer(id, other.c_str());
                return *this;
            }
        };

        LayerProxy layer() { return LayerProxy{m_InstanceID}; }

	};
}

std::ostream& operator<<(std::ostream& os, const CatRuntime::Layer::LayerProxy& proxy);

