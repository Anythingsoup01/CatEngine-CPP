#pragma once

#include "CatScriptCore/Types/Types.h"
#include "InternalObjectData.h"
#include "CatScriptCore/InternalCalls/InternalCalls.h"

namespace CatRuntime
{
    struct SpriteRenderer
    {
    public:
        SpriteRenderer() = default;
        SpriteRenderer(uint64_t entityID) : m_EntityID(entityID) {}

        struct Texture
        {
        public:
            uint64_t id;

            operator Texture2D() const
            {
                Texture2D textureID;
                Sprite_GetTextureID(id, &textureID.ID);
                return textureID;
            }

            Texture& operator=(const Texture2D& t)
            {
                Texture2D tT = t;
                Sprite_SetTextureID(id, &tT.ID);
                return *this;
            }
        };

        Texture texture() { return Texture{m_EntityID}; }

        struct Color
        {
        public:
            uint64_t id;

            operator Vector4() const {
                Vector4 v;
                Sprite_GetColor(id, &v);
                return v;
            }

            Color& operator=(const Vector4& v) {
                Vector4 vT = v;
                Sprite_SetColor(id, &vT);
                return *this;
            }
        };

        Color color() { return Color{m_EntityID}; }
    protected:
        uint64_t m_EntityID = 0;
    };
}
