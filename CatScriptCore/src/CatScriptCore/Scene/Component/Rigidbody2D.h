#pragma once

#include "InternalObjectData.h"

#include "CatScriptCore/Types/Types.h"

#include "CatScriptCore/InternalCalls/InternalCalls.h"

namespace CatRuntime
{
    class Rigidbody2D : InternalObjectData
    {
    public:
        Rigidbody2D(uint64_t entityID) : InternalObjectData(entityID) {}

        void ApplyForce(const Vector2& impulse, const Vector2& point, bool wake = true)
        {
            Vector2 imp = impulse;
            Vector2 poi = point;
            Rigidbody2D_ApplyForce(m_InstanceID, &imp, &poi, wake);
        }
        void ApplyForceToCenter(uint64_t entityID, const Vector2& impulse, bool wake)
        {
            Vector2 imp = impulse;
            Rigidbody2D_ApplyForceToCenter(entityID, &imp, wake);
        }
        void ApplyLinearImpulse(uint64_t entityID, const Vector2& impulse, const Vector2& point, bool wake)
        {
            Vector2 imp = impulse;
            Vector2 poi = point;
            Rigidbody2D_ApplyLinearImpulse(entityID, &imp, &poi, wake);
        }
        void ApplyLinearImpulseToCenter(uint64_t entityID, const Vector2& impulse, bool wake)
        {
            Vector2 imp = impulse;
            Rigidbody2D_ApplyLinearImpulseToCenter(entityID, &imp, wake);
        }
    };
}
