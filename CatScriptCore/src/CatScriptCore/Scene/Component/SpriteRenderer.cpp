#include "SpriteRenderer.h"
#include <cstdint>

namespace CatRuntime {

SpriteRenderer::TextureProxy::operator Texture() const {
  if (m_EntityID == 0)
    return Texture(0);
  uint64_t id;
  SpriteRenderer_GetTextureID(m_EntityID, &id);
  return Texture(id);
}

SpriteRenderer::TextureProxy &SpriteRenderer::TextureProxy::operator=(const Texture &t) {
  if (m_EntityID == 0) {
    return *this;
  }
  uint64_t id = t.GetID();
  SpriteRenderer_SetTextureID(m_EntityID, &id);
  return *this;
}

SpriteRenderer::ColorProxy::operator Vector4() const {
  if (m_EntityID == 0)
    return {0, 0, 0, 0};
  Vector4 v = Vector4{0, 0, 0, 0};
  SpriteRenderer_GetColor(m_EntityID, &v);
  return v;
}

SpriteRenderer::ColorProxy &SpriteRenderer::ColorProxy::operator=(const Vector4 &v) {
  if (m_EntityID == 0) {
    return *this;
  }

  Vector4 vT = v;
  SpriteRenderer_SetColor(m_EntityID, &vT);
  return *this;
}

} // namespace CatRuntime
