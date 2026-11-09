#include "CircleCollider2D.h"

#include "CatScriptCore/InternalCalls/InternalCalls.h"

namespace CatRuntime {

Vector2IndexProxyFloatOperator(
  CircleCollider2D,
  switch(m_Field) {
    case OFFSET:
      CircleCollider2D_GetOffset(m_EntityID, &v);
      break;
    default:
      break;
  }
);

Vector2IndexProxyEqualsOperator(
  CircleCollider2D,
  switch(m_Field) {
    case OFFSET:
      CircleCollider2D_GetOffset(m_EntityID, &v);
      break;
    default:
      break;
  },
  switch(m_Field) {
    case OFFSET:
      CircleCollider2D_SetOffset(m_EntityID, &v);
      break;
    default:
      break;
  }
);

Vector2ProxyVector2Operator(
  CircleCollider2D,
  switch(m_Field) {
    case OFFSET:
      CircleCollider2D_GetOffset(m_EntityID, &v);
      break;
    default:
      break;
  }
);

Vector2ProxyEqualsOperator(
  CircleCollider2D,
  switch(m_Field) {
    case OFFSET:
      CircleCollider2D_SetOffset(m_EntityID, &vT);
      break;
    default:
      break;
  }
);

FloatProxyFloatOperator(
  CircleCollider2D,
  switch (m_Field) {
    case RADIUS:
      CircleCollider2D_GetRadius(m_EntityID, &f);
      break;
    case DENSITY:
      CircleCollider2D_GetDensity(m_EntityID, &f);
      break;
    case FRICTION:
      CircleCollider2D_GetFriction(m_EntityID, &f);
      break;
    case RESTITUTION:
      CircleCollider2D_GetRestitution(m_EntityID, &f);
      break;
    case RESTITUTION_THRESHOLD:
      CircleCollider2D_GetRestitutionThreshold(m_EntityID, &f);
      break;
    default:
      break;
  }
);

FloatProxyEqualsOperator(
  CircleCollider2D,
  switch (m_Field) {
    case RADIUS:
      CircleCollider2D_SetRadius(m_EntityID, &fT);
      break;
    case DENSITY:
      CircleCollider2D_SetDensity(m_EntityID, &fT);
      break;
    case FRICTION:
      CircleCollider2D_SetFriction(m_EntityID, &fT);
      break;
    case RESTITUTION:
      CircleCollider2D_SetRestitution(m_EntityID, &fT);
      break;
    case RESTITUTION_THRESHOLD:
      CircleCollider2D_SetRestitutionThreshold(m_EntityID, &fT);
      break;
    default:
      break;
  }
);

}
