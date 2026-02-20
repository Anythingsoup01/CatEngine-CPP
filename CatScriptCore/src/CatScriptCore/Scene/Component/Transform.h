#pragma once

#include "CatScriptCore/InternalCalls/InternalCalls.h"
#include "CatScriptCore/Types/Types.h"

namespace CatRuntime {
struct Transform {
public:
  uint64_t id;

  struct PositionProxy {
  public:
    uint64_t id;
    // Read whole vector
    operator Vector3() const {
      if (id <= 0)
        return {0, 0, 0};
      Vector3 v;
      Transform_GetPosition(id, &v);
      return v;
    }

    // Write whole vector
    PositionProxy &operator=(const Vector3 &v) {
      if (id <= 0)
        return *this;
      Vector3 vT = v;
      Transform_SetPosition(id, &vT);
      return *this;
    }

    struct FloatProxy {
    public:
      uint64_t id;
      int index; // 0 = x, 1 = y, 2 = z

      operator float() const {
        if (id <= 0)
          return 0;
        Vector3 v;
        Transform_GetPosition(id, &v);
        return (&v.x)[index];
      }

      FloatProxy &operator=(float f) {
        if (id <= 0)
          return *this;
        Vector3 v;
        Transform_GetPosition(id, &v);
        (&v.x)[index] = f;
        Transform_SetPosition(id, &v);
        return *this;
      }
    };

    // Component proxies
    FloatProxy x() { return FloatProxy{id, 0}; }
    FloatProxy y() { return FloatProxy{id, 1}; }
    FloatProxy z() { return FloatProxy{id, 2}; }
  };

  PositionProxy position() { return PositionProxy{id}; }

  struct RotationProxy {
  public:
    uint64_t id;
    // Read whole vector
    operator Vector3() const {
      if (id <= 0)
        return {0, 0, 0};
      Vector3 v;
      Transform_GetRotation(id, &v);
      return v;
    }

    // Write whole vector
    RotationProxy &operator=(const Vector3 &v) {
      if (id <= 0)
        return *this;
      Vector3 vT = v;
      Transform_SetRotation(id, &vT);
      return *this;
    }

    struct FloatProxy {
    public:
      uint64_t id;
      int index; // 0 = x, 1 = y, 2 = z

      operator float() const {
        if (id <= 0)
          return 0;
        Vector3 v;
        Transform_GetPosition(id, &v);
        return (&v.x)[index];
      }

      FloatProxy &operator=(float f) {
        if (id <= 0)
          return *this;
        Vector3 v;
        Transform_GetRotation(id, &v);
        (&v.x)[index] = f;
        Transform_SetRotation(id, &v);
        return *this;
      }
    };

    // Component proxies
    FloatProxy x() { return FloatProxy{id, 0}; }
    FloatProxy y() { return FloatProxy{id, 1}; }
    FloatProxy z() { return FloatProxy{id, 2}; }
  };

  RotationProxy rotation() { return RotationProxy{id}; }
};

} // namespace CatRuntime
