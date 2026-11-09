#pragma once

#include <cstdint>

#include "CatScriptCore/Types/Vector.h"

namespace CatRuntime {

struct CircleRenderer {
protected:
  uint64_t m_EntityID;

private:
  enum Field {
    COLOR, THICKNESS, FADE
  };

  struct Vector4IndexProxy {
  public:
    Vector4IndexProxy(const uint64_t &entityID, const int &index, const Field &field)
      : m_EntityID(entityID), m_Index(index), m_Field(field) {}

    operator float();
    Vector4IndexProxy &operator=(const float &f);
  private:
    uint64_t m_EntityID;
    int m_Index;
    Field m_Field;
  };

  struct Vector4Proxy {
  public:
    Vector4Proxy(const uint64_t &entityID, const Field &field)
      : m_EntityID(entityID), m_Field(field) {}

    operator Vector4() const;
    Vector4Proxy &operator=(const Vector4 &v);

    Vector4IndexProxy x() { return Vector4IndexProxy(m_EntityID, 0, m_Field); }
    Vector4IndexProxy y() { return Vector4IndexProxy(m_EntityID, 1, m_Field); }
    Vector4IndexProxy z() { return Vector4IndexProxy(m_EntityID, 2, m_Field); }
    Vector4IndexProxy a() { return Vector4IndexProxy(m_EntityID, 3, m_Field); }
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
  CircleRenderer(const uint64_t &entityID)
    : m_EntityID(entityID) {}

  Vector4Proxy offset() { return Vector4Proxy(m_EntityID, COLOR); }

  FloatProxy thickness()  { return FloatProxy(m_EntityID, THICKNESS); }
  FloatProxy fade()       { return FloatProxy(m_EntityID, FADE);      }

};

} // namespace CatRuntime
