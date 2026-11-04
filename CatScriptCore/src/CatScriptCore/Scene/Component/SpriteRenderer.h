#pragma once

#include "CatScriptCore/InternalCalls/InternalCalls.h"
#include "CatScriptCore/Types/Texture.h"
#include "CatScriptCore/Types/Types.h"
#include "InternalObjectData.h"

namespace CatRuntime {
struct SpriteRenderer {
public:
  SpriteRenderer() = default;
  SpriteRenderer(uint64_t entityID) : m_EntityID(entityID) {}

  struct Texture {
  public:
    uint64_t id;

    operator Texture2D() const {
      if (id == 0)
        return Texture2D{0};
      Texture2D textureID;
      Sprite_GetTextureID(id, &textureID.ID);
      return textureID;
    }

    Texture &operator=(const Texture2D &t) {
      if (id == 0) {
        return *this;
      }
      Texture2D tT = t;
      Sprite_SetTextureID(id, &tT.ID);
      return *this;
    }
  };

  Texture texture() { return Texture{m_EntityID}; }

  struct Color {
  public:
    uint64_t id;

    operator Vector4() const {
      if (id == 0)
        return {0,0,0,0};
      Vector4 v = Vector4{0,0,0,0};
      Sprite_GetColor(id, &v);
      return v;
    }

    Color &operator=(const Vector4 &v) {
      if (id == 0) {
        return *this;
      }

      Vector4 vT = v;
      Sprite_SetColor(id, &vT);
      return *this;
    }
  };

  Color color() { return Color{m_EntityID}; }

  operator bool() {return m_EntityID != 0;}

protected:
  uint64_t m_EntityID = 0;
};
} // namespace CatRuntime
