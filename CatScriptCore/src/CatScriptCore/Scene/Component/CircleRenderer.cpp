#include "CircleRenderer.h"
#include "CatScriptCore/InternalCalls/InternalCalls.h"

namespace CatRuntime {
Vector4IndexProxyFloatOperator(
  CircleRenderer,
  switch(m_Field) {
    case COLOR:
      CircleRenderer_GetColor(m_EntityID, &v);
      break;
    default:
      break;
  }
);

Vector4IndexProxyEqualsOperator(
  CircleRenderer,
  switch(m_Field) {
    case COLOR:
      CircleRenderer_GetColor(m_EntityID, &v);
      break;
    default:
      break;
  },
  switch(m_Field) {
    case COLOR:
      CircleRenderer_SetColor(m_EntityID, &v);
      break;
    default:
      break;
  }
);

Vector4ProxyVector4Operator(
  CircleRenderer,
  switch(m_Field) {
    case COLOR:
      CircleRenderer_GetColor(m_EntityID, &v);
      break;
    default:
      break;
  }
);

Vector4ProxyEqualsOperator(
  CircleRenderer,
  switch(m_Field) {
    case COLOR:
      CircleRenderer_SetColor(m_EntityID, &vT);
      break;
    default:
      break;
  }
);

FloatProxyFloatOperator(
  CircleRenderer,
  switch (m_Field) {
    case THICKNESS:
      CircleRenderer_GetThickness(m_EntityID, &f);
      break;
    case FADE:
      CircleRenderer_GetFade(m_EntityID, &f);
      break;
    default:
      break;
  }
);

FloatProxyEqualsOperator(
  CircleRenderer,
  switch (m_Field) {
    case THICKNESS:
      CircleRenderer_SetThickness(m_EntityID, &fT);
      break;
    case FADE:
      CircleRenderer_SetFade(m_EntityID, &fT);
      break;
    default:
      break;
  }
);
}
