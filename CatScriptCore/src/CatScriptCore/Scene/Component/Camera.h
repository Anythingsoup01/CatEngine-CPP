#pragma once

#include "CatScriptCore/Scene/Component/ComponentHelper.h"
#include <cstdint>
namespace CatRuntime {

struct Camera {
protected:
  uint64_t m_EntityID;

private:
  enum Field {
    SIZE, FOV,
    NEAR, FAR,
    ASPECT
  };

  FloatProxyDefinition();

public:
  Camera(const uint64_t &entityID)
    : m_EntityID(entityID) {}

  void SetOrthographic();
  void SetPerspective();

  FloatProxy size()           { return FloatProxy(m_EntityID, SIZE);     }
  FloatProxy fov()            { return FloatProxy(m_EntityID, FOV);      }
  FloatProxy near()           { return FloatProxy(m_EntityID, NEAR);     }
  FloatProxy far()            { return FloatProxy(m_EntityID, FAR);      }
  FloatProxy aspect_ratio()   { return FloatProxy(m_EntityID, ASPECT);   }
};

}
