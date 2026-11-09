#include "BoxCollider2D.h"
#include "CatScriptCore/InternalCalls/InternalCalls.h"
#include "CatScriptCore/Scene/Component/ComponentHelper.h"

namespace CatRuntime {

Vector2IndexProxyFloatOperator(
  BoxCollider2D,
  switch(m_Field) {
    case OFFSET:
      BoxCollider2D_GetOffset(m_EntityID, &v);
      break;
    case SIZE:
      BoxCollider2D_GetSize(m_EntityID, &v);
      break;
    default:
      break;
  }
);

Vector2IndexProxyEqualsOperator(
  BoxCollider2D,
  switch(m_Field) {
    case OFFSET:
      BoxCollider2D_GetOffset(m_EntityID, &v);
      break;
    case SIZE:
      BoxCollider2D_GetSize(m_EntityID, &v);
      break;
    default:
      break;
  },
  switch(m_Field) {
    case OFFSET:
      BoxCollider2D_SetOffset(m_EntityID, &v);
      break;
    case SIZE:
      BoxCollider2D_SetSize(m_EntityID, &v);
      break;
    default:
      break;
  }
);

Vector2ProxyVector2Operator(
  BoxCollider2D,
  switch(m_Field) {
    case OFFSET:
      BoxCollider2D_GetOffset(m_EntityID, &v);
      break;
    case SIZE:
      BoxCollider2D_GetSize(m_EntityID, &v);
      break;
    default:
      break;
  }
);

Vector2ProxyEqualsOperator(
  BoxCollider2D,
  switch(m_Field) {
    case OFFSET:
      BoxCollider2D_SetOffset(m_EntityID, &vT);
      break;
    case SIZE:
      BoxCollider2D_SetSize(m_EntityID, &vT);
      break;
    default:
      break;
  }
);

FloatProxyFloatOperator(
  BoxCollider2D,
  switch (m_Field) {
    case ROTATION:
      BoxCollider2D_GetRotation(m_EntityID, &f);
      break;
    case DENSITY:
      BoxCollider2D_GetDensity(m_EntityID, &f);
      break;
    case FRICTION:
      BoxCollider2D_GetFriction(m_EntityID, &f);
      break;
    case RESTITUTION:
      BoxCollider2D_GetRestitution(m_EntityID, &f);
      break;
    case RESTITUTION_THRESHOLD:
      BoxCollider2D_GetRestitutionThreshold(m_EntityID, &f);
      break;
    default:
      break;
  }
)

FloatProxyEqualsOperator(
  BoxCollider2D,
  switch (m_Field) {
    case ROTATION:
      BoxCollider2D_SetRotation(m_EntityID, &fT);
      break;
    case DENSITY:
      BoxCollider2D_SetDensity(m_EntityID, &fT);
      break;
    case FRICTION:
      BoxCollider2D_SetFriction(m_EntityID, &fT);
      break;
    case RESTITUTION:
      BoxCollider2D_SetRestitution(m_EntityID, &fT);
      break;
    case RESTITUTION_THRESHOLD:
      BoxCollider2D_SetRestitutionThreshold(m_EntityID, &fT);
      break;
    default:
      break;
  }
)

} // namespace CatRuntime
