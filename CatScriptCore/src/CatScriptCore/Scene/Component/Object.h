#pragma once

#include <cstdint>
#include "Transform.h"

namespace CatRuntime
{

	class Object : InternalObjectData
	{
    public:
        Object(uint64_t entityID) : transform(entityID), InternalObjectData(entityID) {}
        Transform transform;

        template<typename T>
        bool HasComponent();

        template<typename T>
        T GetComponent();

    protected :


    private:

    };

}
