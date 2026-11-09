#pragma once

#include <cstdint>

#include "CatScriptCore/Scene/Component/ComponentHelper.h"
#include "CatScriptCore/Types/Vector.h"

namespace CatRuntime {

struct CircleRenderer {
protected:
  uint64_t m_EntityID;

private:
  enum Field {
    COLOR, THICKNESS, FADE
  };

  Vector4IndexProxyDefinition();

  Vector4ProxyDefinition();

  FloatProxyDefinition();
public:
  CircleRenderer(const uint64_t &entityID)
    : m_EntityID(entityID) {}

  Vector4Proxy offset() { return Vector4Proxy(m_EntityID, COLOR); }

  FloatProxy thickness()  { return FloatProxy(m_EntityID, THICKNESS); }
  FloatProxy fade()       { return FloatProxy(m_EntityID, FADE);      }

};

} // namespace CatRuntime
