#pragma once

#include <cstdint>
#include "Transform.h"
#include "Tag.h"
#include "Layer.h"

namespace CatRuntime
{

	class Object
	{
    protected:
        uint64_t m_EntityID = 0;
    public:
        Object() = default;
        Object(uint64_t entityID) : m_EntityID(entityID) {}

        Transform transform() { return Transform{m_EntityID}; }

        Tag tag() { return Tag{m_EntityID}; }

        Layer layer() { return Layer{m_EntityID}; }

        template<typename T>
        bool HasComponent();

        template<typename T>
        T GetComponent();

    };
}
