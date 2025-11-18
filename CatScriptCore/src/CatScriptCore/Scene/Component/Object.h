#pragma once

#include <cstdint>
#include "Transform.h"
#include "Tag.h"
#include "Layer.h"

namespace CatRuntime
{

	class Object : InternalObjectData
	{
    public:
        Object(uint64_t entityID) : transform(entityID), tag(entityID), layer(entityID), InternalObjectData(entityID) {}
        Transform transform;
        Tag tag;
        Layer layer;

        template<typename T>
        bool HasComponent();

        template<typename T>
        T GetComponent();

    protected :


    private:

    };

}
