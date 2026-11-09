#pragma once

#include "CatScriptCore/InternalCalls/InternalCalls.h"
#include "CatScriptCore/Types/Texture.h"
#include "CatScriptCore/Types/Types.h"
#include <cstdint>

namespace CatRuntime {
struct SpriteRenderer {
protected:
  uint64_t m_EntityID = 0;

private:
  struct TextureProxy {
  public:
    TextureProxy(const uint64_t &entityID) : m_EntityID(entityID) {}

    operator Texture2D() const;

    TextureProxy &operator=(const Texture2D &t);

  private:
    uint64_t m_EntityID;
  };

  struct ColorProxy {
  public:
    ColorProxy(const uint64_t &entityID) : m_EntityID(entityID) {}

    operator Vector4() const;

    ColorProxy &operator=(const Vector4 &v);

  private:
    uint64_t m_EntityID;
  };

public:
  SpriteRenderer(uint64_t entityID) : m_EntityID(entityID) {}

  TextureProxy texture() { return TextureProxy(m_EntityID); }
  ColorProxy color() { return ColorProxy(m_EntityID); }
};
} // namespace CatRuntime
