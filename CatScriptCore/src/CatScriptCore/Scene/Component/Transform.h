#pragma once

#include "CatScriptCore/Types/Types.h"

namespace CatRuntime
{
	class TransformComponent
	{
    public:
        Vector3 Position();
        void Position(const Vector3& position);
        void Position(float x, float y, float z);

        Vector3 Rotation();


        Vector3 Scale();
	};
}
