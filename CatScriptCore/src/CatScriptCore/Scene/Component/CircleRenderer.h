#pragma once

#include "CatScriptCore/Types/Types.h"
#include "InternalObjectData.h"

namespace CatRuntime
{
	struct CircleRenderer
	{
    public:
        CircleRenderer() = default;
        CircleRenderer(uint64_t entityID) { m_EntityID = entityID; }
    protected:
        uint64_t m_EntityID;
	};
}
