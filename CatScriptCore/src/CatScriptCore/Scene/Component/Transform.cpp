#include "Transform.h"

#include "CatScriptCore/InternalCalls/InternalCalls.h"

namespace CatRuntime {

Vector3IndexProxyFloatOperator(
  Transform,
  switch(m_Field) {
    case WORLD_POSITION:
      Transform_GetPosition(m_EntityID, &v);
      break;
    case WORLD_ROTATION:
      Transform_GetRotation(m_EntityID, &v);
      break;
    case WORLD_SCALE:
      Transform_GetScale(m_EntityID, &v);
      break;
    case LOCAL_POSITION:
      Transform_GetLocalPosition(m_EntityID, &v);
      break;
    case LOCAL_ROTATION:
      Transform_GetLocalRotation(m_EntityID, &v);
      break;
    case LOCAL_SCALE:
      Transform_GetLocalScale(m_EntityID, &v);
      break;
    default:
      break;
  }
);

Vector3IndexProxyEqualsOperator(
  Transform,
  switch(m_Field) {
    case WORLD_POSITION:
      Transform_GetPosition(m_EntityID, &v);
      break;
    case WORLD_ROTATION:
      Transform_GetRotation(m_EntityID, &v);
      break;
    case WORLD_SCALE:
      Transform_GetScale(m_EntityID, &v);
      break;
    case LOCAL_POSITION:
      Transform_GetLocalPosition(m_EntityID, &v);
      break;
    case LOCAL_ROTATION:
      Transform_GetLocalRotation(m_EntityID, &v);
      break;
    case LOCAL_SCALE:
      Transform_GetLocalScale(m_EntityID, &v);
      break;
    default:
      break;
  },
  switch(m_Field) {
    case WORLD_POSITION:
      Transform_SetPosition(m_EntityID, &v);
      break;
    case WORLD_ROTATION:
      Transform_SetRotation(m_EntityID, &v);
      break;
    case WORLD_SCALE:
      Transform_SetScale(m_EntityID, &v);
      break;
    case LOCAL_POSITION:
      Transform_SetLocalPosition(m_EntityID, &v);
      break;
    case LOCAL_ROTATION:
      Transform_SetLocalRotation(m_EntityID, &v);
      break;
    case LOCAL_SCALE:
      Transform_SetLocalScale(m_EntityID, &v);
      break;
    default:
      break;
  }
);

Vector3ProxyVector3Operator(
  Transform,
  switch(m_Field) {
    case WORLD_POSITION:
      Transform_GetPosition(m_EntityID, &v);
      break;
    case WORLD_ROTATION:
      Transform_GetRotation(m_EntityID, &v);
      break;
    case WORLD_SCALE:
      Transform_GetScale(m_EntityID, &v);
      break;
    case LOCAL_POSITION:
      Transform_GetLocalPosition(m_EntityID, &v);
      break;
    case LOCAL_ROTATION:
      Transform_GetLocalRotation(m_EntityID, &v);
      break;
    case LOCAL_SCALE:
      Transform_GetLocalScale(m_EntityID, &v);
      break;
    default:
      break;
  }
);

Vector3ProxyEqualsOperator(
  Transform,
  switch(m_Field) {
    case WORLD_POSITION:
      Transform_SetPosition(m_EntityID, &vT);
      break;
    case WORLD_ROTATION:
      Transform_SetRotation(m_EntityID, &vT);
      break;
    case WORLD_SCALE:
      Transform_SetScale(m_EntityID, &vT);
      break;
    case LOCAL_POSITION:
      Transform_SetLocalPosition(m_EntityID, &vT);
      break;
    case LOCAL_ROTATION:
      Transform_SetLocalRotation(m_EntityID, &vT);
      break;
    case LOCAL_SCALE:
      Transform_SetLocalScale(m_EntityID, &vT);
      break;
    default:
      break;
  }
);

}
