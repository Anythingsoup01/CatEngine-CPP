#pragma once

#include "CatScriptCore/Scene/Component/Object.h"
#include "CatScriptCore/Types/Types.h"

namespace CatRuntime
{
	class Transform : public Object
	{
    public:
        Transform() = default;
        Transform(uint64_t entityID) { m_InstanceID = entityID; }
    
        Vector3 Position();
        void Position(const Vector3& pos);

	};
}
