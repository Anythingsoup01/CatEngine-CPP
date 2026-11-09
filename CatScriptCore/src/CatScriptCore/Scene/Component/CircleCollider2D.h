#pragma once

#include <cstdint>

#include "CatScriptCore/Types/Vector.h"

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

  struct Vector2IndexProxy {
  public:
    Vector2IndexProxy(const uint64_t &entityID, const int &index, const Field &field)
      : m_EntityID(entityID), m_Index(index), m_Field(field) {}

    operator float();
    Vector2IndexProxy &operator=(const float &f);
  private:
    uint64_t m_EntityID;
    int m_Index;
    Field m_Field;
  };

  struct Vector2Proxy {
  public:
    Vector2Proxy(const uint64_t &entityID, const Field &field)
      : m_EntityID(entityID), m_Field(field) {}

    operator Vector2() const;
    Vector2Proxy &operator=(const Vector2 &v);

    Vector2IndexProxy x() { return Vector2IndexProxy(m_EntityID, 0, m_Field); }
    Vector2IndexProxy y() { return Vector2IndexProxy(m_EntityID, 1, m_Field); }
  private:
    uint64_t m_EntityID;
    Field m_Field;
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

