#pragma once

#include <cstdint>

#include "CatScriptCore/Scene/Component/ComponentHelper.h"

namespace CatRuntime {

struct CircleCollider2D {
protected:
  uint64_t m_EntityID;

private:
  enum Field {
    OFFSET,
    RADIUS, DENSITY, FRICTION,
    RESTITUTION, RESTITUTION_THRESHOLD
  };

  Vector2IndexProxyDefinition();

  Vector2ProxyDefinition();

  FloatProxyDefinition();

public:
  CircleCollider2D(const uint64_t &entityID)
    : m_EntityID(entityID) {}

  Vector2Proxy offset() { return Vector2Proxy(m_EntityID, OFFSET); }

  FloatProxy radius() { return FloatProxy(m_EntityID, RADIUS); }
  FloatProxy density()  { return FloatProxy(m_EntityID, DENSITY);  }
  FloatProxy friction() { return FloatProxy(m_EntityID, FRICTION); }

  FloatProxy restitution()            { return FloatProxy(m_EntityID, RESTITUTION);           }
  FloatProxy restitution_threshold()  { return FloatProxy(m_EntityID, RESTITUTION_THRESHOLD); }
};

}

