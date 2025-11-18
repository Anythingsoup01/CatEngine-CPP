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
        Rigidbody2D() = default;

        void ApplyForce(const Vector2& impulse, const Vector2& point, bool wake = true)
        {
            Vector2 imp = impulse;
            Vector2 poi = point;
            Rigidbody2D_ApplyForce(m_InstanceID, &imp, &poi, wake);
        }
        void ApplyForceToCenter(const Vector2& impulse, bool wake = true)
        {
            Vector2 imp = impulse;
            Rigidbody2D_ApplyForceToCenter(m_InstanceID, &imp, wake);
        }
        void ApplyLinearImpulse(const Vector2& impulse, const Vector2& point, bool wake = true)
        {
            Vector2 imp = impulse;
            Vector2 poi = point;
            Rigidbody2D_ApplyLinearImpulse(m_InstanceID, &imp, &poi, wake);
        }
        void ApplyLinearImpulseToCenter(const Vector2& impulse, bool wake = true)
        {
            Vector2 imp = impulse;
            Rigidbody2D_ApplyLinearImpulseToCenter(m_InstanceID, &imp, wake);
        }
    };
}
