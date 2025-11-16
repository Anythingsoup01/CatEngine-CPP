#pragma once

#include "CatScriptCore/Types/Types.h"
#include "InternalObjectData.h"

namespace CatRuntime
{
	class CircleRenderer : public InternalObjectData
	{
    public:
        CircleRenderer() = default;
        CircleRenderer(uint64_t entityID) { m_InstanceID = entityID; }
    protected:
	};
}
