#include "SpriteRenderer.h"

namespace CatRuntime {

SpriteRenderer::TextureProxy::operator Texture2D() const {
  if (m_EntityID == 0)
    return Texture2D{0};
  Texture2D textureID;
  SpriteRenderer_GetTextureID(m_EntityID, &textureID.ID);
  return textureID;
}

SpriteRenderer::TextureProxy &SpriteRenderer::TextureProxy::operator=(const Texture2D &t) {
  if (m_EntityID == 0) {
    return *this;
  }
  Texture2D tT = t;
  SpriteRenderer_SetTextureID(m_EntityID, &tT.ID);
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
