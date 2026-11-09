#pragma once

#include <cstdint>

#include "ComponentHelper.h"

namespace CatRuntime {

struct BoxCollider2D {
protected:
  uint64_t m_EntityID;

private:
  enum Field {
    OFFSET, SIZE,
    ROTATION, DENSITY, FRICTION,
    RESTITUTION, RESTITUTION_THRESHOLD
  };

  Vector2IndexProxyDefinition();

  Vector2ProxyDefinition();

  FloatProxyDefinition();
public:
  BoxCollider2D(const uint64_t &entityID)
    : m_EntityID(entityID) {}

  Vector2Proxy offset() { return Vector2Proxy(m_EntityID, OFFSET); }
  Vector2Proxy size()   { return Vector2Proxy(m_EntityID, SIZE);   }

  FloatProxy rotation() { return FloatProxy(m_EntityID, ROTATION); }
  FloatProxy density()  { return FloatProxy(m_EntityID, DENSITY);  }
  FloatProxy friction() { return FloatProxy(m_EntityID, FRICTION); }

  FloatProxy restitution()            { return FloatProxy(m_EntityID, RESTITUTION);           }
  FloatProxy restitution_threshold()  { return FloatProxy(m_EntityID, RESTITUTION_THRESHOLD); }
};

}
