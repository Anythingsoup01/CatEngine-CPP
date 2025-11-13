#pragma once

#include "CatScriptCore/Scene/Component/Object.h"
#include "CatScriptCore/Types/Types.h"

namespace CatRuntime
{
	class CircleRenderer : public Object
	{
    public:
        CircleRenderer() = default;
        CircleRenderer(uint64_t entityID) { m_InstanceID = entityID; }
    protected:
	};
}
