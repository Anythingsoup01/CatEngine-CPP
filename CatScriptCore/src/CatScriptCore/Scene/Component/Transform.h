#pragma once

#include "CatScriptCore/Scene/Component/ComponentHelper.h"
#include <cstdint>

namespace CatRuntime {
struct Transform {
protected:
  uint64_t m_EntityID;

private:
  enum Field {
    WORLD_POSITION, WORLD_ROTATION, WORLD_SCALE,
    LOCAL_POSITION, LOCAL_ROTATION, LOCAL_SCALE
  };

  Vector3IndexProxyDefinition();
  Vector3ProxyDefinition();
public:
  Transform(const uint64_t entityID) : m_EntityID(entityID) {}

  Vector3Proxy position() { return Vector3Proxy(m_EntityID, WORLD_POSITION); }
  Vector3Proxy rotation() { return Vector3Proxy(m_EntityID, WORLD_ROTATION); }
  Vector3Proxy scale()    { return Vector3Proxy(m_EntityID, WORLD_SCALE); }

  Vector3Proxy local_position() { return Vector3Proxy(m_EntityID, LOCAL_POSITION); }
  Vector3Proxy local_rotation() { return Vector3Proxy(m_EntityID, LOCAL_ROTATION); }
  Vector3Proxy local_scale()    { return Vector3Proxy(m_EntityID, LOCAL_SCALE); }
};

} // namespace CatRuntime
