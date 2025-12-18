#pragma once

#include "InternalObjectData.h"

#include "CatScriptCore/Types/Types.h"

#include "CatScriptCore/InternalCalls/InternalCalls.h"

namespace CatRuntime
{
    struct Rigidbody2D
    {
    public:
        Rigidbody2D() = default;
        Rigidbody2D(uint64_t entityID) : m_EntityID(entityID) {}

        void ApplyForce(const Vector2& impulse, const Vector2& point, bool wake = true)
        {
            Vector2 imp = impulse;
            Vector2 poi = point;
            Rigidbody2D_ApplyForce(m_EntityID, &imp, &poi, wake);
        }
        void ApplyForceToCenter(const Vector2& impulse, bool wake = true)
        {
            Vector2 imp = impulse;
            Rigidbody2D_ApplyForceToCenter(m_EntityID, &imp, wake);
        }
        void ApplyLinearImpulse(const Vector2& impulse, const Vector2& point, bool wake = true)
        {
            Vector2 imp = impulse;
            Vector2 poi = point;
            Rigidbody2D_ApplyLinearImpulse(m_EntityID, &imp, &poi, wake);
        }
        void ApplyLinearImpulseToCenter(const Vector2& impulse, bool wake = true)
        {
            Vector2 imp = impulse;
            Rigidbody2D_ApplyLinearImpulseToCenter(m_EntityID, &imp, wake);
        }
    private:
        uint64_t m_EntityID = 0;
    };
}
