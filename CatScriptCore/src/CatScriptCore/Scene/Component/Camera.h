#pragma once

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

  struct FloatProxy {
  public:
    FloatProxy(const uint64_t &entityID, const Field &field)
      : m_EntityID(entityID), m_Field(field) {}

    operator float() const;
    FloatProxy &operator=(const float &f);
  private:
    uint64_t m_EntityID;
    Field m_Field;
  };

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
