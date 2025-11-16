#pragma once

#include "CatScriptCore/Types/Types.h"
#include "CatScriptCore/InternalCalls/InternalCalls.h"
#include "InternalObjectData.h"

namespace CatRuntime
{
	class Transform : InternalObjectData
    {
    public:
        Transform() = default;
        Transform(uint64_t entityID) : InternalObjectData(entityID) {}

        uint64_t GetID() const { return m_InstanceID; };
        // -------- FloatProxy (for .x/.y/.z) --------
        class FloatProxy {
            public:
                uint64_t id;
                int index; // 0 = x, 1 = y, 2 = z

                operator float() const
                {
                    if (id <= 0) return 0;
                    Vector3 v;
                    Transform_GetPosition(id, &v);
                    return (&v.x)[index];
                }

                FloatProxy& operator=(float f)
                {
                    if (id <= 0) return *this;
                    Vector3 v;
                    Transform_GetPosition(id, &v);
                    (&v.x)[index] = f;
                    Transform_SetPosition(id, &v);
                    return *this;
                }
        };

        // -------- VectorProxy (for .position) --------
        class Vector3Proxy {
            public:
                uint64_t id;
                // Read whole vector
                operator Vector3() const {
                    if (id <= 0) return {0, 0, 0};
                    Vector3 v;
                    Transform_GetPosition(id, &v);
                    return v;
                }

                // Write whole vector
                Vector3Proxy& operator=(const Vector3& v)
                {
                    if (id <= 0) return *this;
                    Vector3 vT = v;
                    Transform_SetPosition(id, &vT);
                    return *this;
                }

                // Component proxies
                FloatProxy x() { return FloatProxy{id, 0}; }
                FloatProxy y() { return FloatProxy{id, 1}; }
                FloatProxy z() { return FloatProxy{id, 2}; }
        };

        // -------- Getter returns VectorProxy --------
        Vector3Proxy position() { return Vector3Proxy{m_InstanceID}; }
    };
}
