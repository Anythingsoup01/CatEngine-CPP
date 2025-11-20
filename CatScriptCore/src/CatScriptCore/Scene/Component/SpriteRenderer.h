#pragma once

#include "CatScriptCore/Types/Types.h"
#include "InternalObjectData.h"
#include "CatScriptCore/InternalCalls/InternalCalls.h"

namespace CatRuntime
{
    class SpriteRenderer : InternalObjectData
    {
    public:
        SpriteRenderer() = default;
        SpriteRenderer(uint64_t entityID) { m_InstanceID = entityID; }

        class TextureProxy
        {
        public:
            uint64_t id;

            operator Texture2D() const
            {
                Texture2D textureID;
                Sprite_GetTextureID(id, &textureID.ID);
                return textureID;
            }

            TextureProxy& operator=(const Texture2D& t)
            {
                Texture2D tT = t;
                Sprite_SetTextureID(id, &tT.ID);
                return *this;
            }
        };

        TextureProxy texture() { return TextureProxy{m_InstanceID}; }

        class ColorProxy
        {
        public:
            uint64_t id;

            operator Vector4() const {
                Vector4 v;
                Sprite_GetColor(id, &v);
                return v;
            }

            ColorProxy& operator=(const Vector4& v) {
                Vector4 vT = v;
                Sprite_SetColor(id, &vT);
                return *this;
            }
        };

        ColorProxy color() { return ColorProxy{m_InstanceID}; }
    };
}
