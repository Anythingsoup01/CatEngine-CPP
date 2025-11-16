#pragma once

#include "CatScriptCore/Types/Types.h"
#include "InternalObjectData.h"
#include "CatScriptCore/InternalCalls/InternalCalls.h"

namespace CatRuntime
{
    class SpriteRenderer : InternalObjectData
    {
    public:
        SpriteRenderer(uint64_t entityID) { m_InstanceID = entityID; }
        auto color(){
            struct Proxy
            {
                SpriteRenderer* sr;

                operator Vector4() const {
                    Vector4 v;
                    Sprite_GetColor(sr->m_InstanceID, &v);
                    return v;
                }

                Proxy& operator=(const Vector4& v) {
                    Vector4 vT = v;
                    Sprite_SetColor(sr->m_InstanceID, &vT);
                    return *this;
                }
            };
            return Proxy{this};
        }
    };
}
