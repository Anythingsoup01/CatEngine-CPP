#include "Camera.h"

#include "CatScriptCore/InternalCalls/InternalCalls.h"

namespace CatRuntime {

FloatProxyFloatOperator(
  Camera,
  switch (m_Field) {
    case SIZE:
      Camera_GetSize(m_EntityID, &f);
      break;
    case FOV:
      Camera_GetFOV(m_EntityID, &f);
      break;
    case NEAR:
      Camera_GetNear(m_EntityID, &f);
      break;
    case FAR:
      Camera_GetFar(m_EntityID, &f);
      break;
    case ASPECT:
      Camera_GetAspect(m_EntityID, &f);
      break;
    default:
      break;
  }
);

FloatProxyEqualsOperator(
  Camera,
  switch (m_Field) {
    case SIZE:
      Camera_SetSize(m_EntityID, &fT);
      break;
    case FOV:
      Camera_SetFOV(m_EntityID, &fT);
      break;
    case NEAR:
      Camera_SetNear(m_EntityID, &fT);
      break;
    case FAR:
      Camera_SetFar(m_EntityID, &fT);
      break;
    case ASPECT:
      Camera_SetAspect(m_EntityID, &fT);
      break;
    default:
      break;
  }
);

void Camera::SetOrthographic() {
  if (m_EntityID == 0)
    return;

  Camera_SetOrthographic(m_EntityID);
}

void Camera::SetPerspective() {
  if (m_EntityID == 0)
    return;

  Camera_SetPerspective(m_EntityID);
}


}
