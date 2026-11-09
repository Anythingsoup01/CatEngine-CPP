#pragma once

#include "CatScriptCore/Types/Vector.h"

#define Vector4IndexProxyDefinition()                                          \
  struct Vector4IndexProxy {                                                   \
  public:                                                                      \
    Vector4IndexProxy(const uint64_t &entityID, const int &index,              \
                      const Field &field)                                      \
        : m_EntityID(entityID), m_Index(index), m_Field(field) {}              \
    operator float();                                                          \
    Vector4IndexProxy &operator=(const float &f);                              \
                                                                               \
  private:                                                                     \
    uint64_t m_EntityID;                                                       \
    int m_Index;                                                               \
    Field m_Field;                                                             \
  };

#define Vector4IndexProxyFloatOperator(Component, SwitchFunc)                  \
  Component::Vector4IndexProxy::operator float() {                             \
    if (m_EntityID == 0)                                                       \
      return 0;                                                                \
    float f;                                                                   \
    SwitchFunc;                                                                \
    return f;                                                                  \
  }

#define Vector4IndexProxyEqualsOperator(Component, SwitchFunc)                 \
  Component::Vector4IndexProxy &Component::Vector4IndexProxy::operator=(       \
      const float &f) {                                                        \
    if (m_EntityID == 0)                                                       \
      return *this;                                                            \
    float fT = f;                                                              \
    SwitchFunc return *this                                                    \
  }

#define Vector4ProxyDefinition()                                               \
  struct Vector4Proxy {                                                        \
  public:                                                                      \
    Vector4Proxy(const uint64_t &entityID, const Field &field)                 \
        : m_EntityID(entityID), m_Field(field) {}                              \
    operator Vector4() const;                                                  \
    Vector4Proxy &operator=(const Vector4 &v);                                 \
    Vector4IndexProxy x() {                                                    \
      return Vector4IndexProxy(m_EntityID, 0, m_Field);                        \
    }                                                                          \
    Vector4IndexProxy y() {                                                    \
      return Vector4IndexProxy(m_EntityID, 1, m_Field);                        \
    }                                                                          \
    Vector4IndexProxy z() {                                                    \
      return Vector4IndexProxy(m_EntityID, 2, m_Field);                        \
    }                                                                          \
    Vector4IndexProxy a() {                                                    \
      return Vector4IndexProxy(m_EntityID, 3, m_Field);                        \
    }                                                                          \
                                                                               \
  private:                                                                     \
    uint64_t m_EntityID;                                                       \
    Field m_Field;                                                             \
  };

#define Vector4ProxyVector4Operator(Component, SwitchFunc)                     \
  Component::Vector4Proxy::operator Vector4() {                                \
    if (m_EntityID == 0)                                                       \
      return 0;                                                                \
    Vector4 v;                                                                 \
    SwitchFunc;                                                                \
    return v;                                                                  \
  }

#define Vector4ProxyEqualsOperator(Component, SwitchFunc)                      \
  Component::Vector4IndexProxy &Component::Vector4Proxy::operator=(            \
      const Vector4 &v) {                                                      \
    if (m_EntityID == 0)                                                       \
      return *this;                                                            \
    Vector4 vT = v;                                                            \
    SwitchFunc;                                                                \
    return *this                                                               \
  }

#define Vector3IndexProxyDefinition()                                          \
  struct Vector3IndexProxy {                                                   \
  public:                                                                      \
    Vector3IndexProxy(const uint64_t &entityID, const int &index,              \
                      const Field &field)                                      \
        : m_EntityID(entityID), m_Index(index), m_Field(field) {}              \
    operator float();                                                          \
    Vector3IndexProxy &operator=(const float &f);                              \
                                                                               \
  private:                                                                     \
    uint64_t m_EntityID;                                                       \
    int m_Index;                                                               \
    Field m_Field;                                                             \
  };

#define Vector3IndexProxyFloatOperator(Component, SwitchFunc)                  \
  Component::Vector3IndexProxy::operator float() {                             \
    if (m_EntityID == 0)                                                       \
      return 0;                                                                \
    float f;                                                                   \
    SwitchFunc;                                                                \
    return f;                                                                  \
  }

#define Vector3IndexProxyEqualsOperator(Component, SwitchFunc)                 \
  Component::Vector3IndexProxy &Component::Vector3IndexProxy::operator=(       \
      const float &f) {                                                        \
    if (m_EntityID == 0)                                                       \
      return *this;                                                            \
    float fT = f;                                                              \
    SwitchFunc return *this                                                    \
  }

#define Vector3ProxyDefinition()                                               \
  struct Vector3Proxy {                                                        \
  public:                                                                      \
    Vector3Proxy(const uint64_t &entityID, const Field &field)                 \
        : m_EntityID(entityID), m_Field(field) {}                              \
    operator Vector3() const;                                                  \
    Vector3Proxy &operator=(const Vector3 &v);                                 \
    Vector3IndexProxy x() {                                                    \
      return Vector3IndexProxy(m_EntityID, 0, m_Field);                        \
    }                                                                          \
    Vector3IndexProxy y() {                                                    \
      return Vector3IndexProxy(m_EntityID, 1, m_Field);                        \
    }                                                                          \
    Vector3IndexProxy z() {                                                    \
      return Vector3IndexProxy(m_EntityID, 2, m_Field);                        \
    }                                                                          \
                                                                               \
  private:                                                                     \
    uint64_t m_EntityID;                                                       \
    Field m_Field;                                                             \
  };

#define Vector3ProxyVector3Operator(Component, SwitchFunc)                     \
  Component::Vector3Proxy::operator Vector3() {                                \
    if (m_EntityID == 0)                                                       \
      return 0;                                                                \
    Vector3 v;                                                                 \
    SwitchFunc;                                                                \
    return v;                                                                  \
  }

#define Vector3ProxyEqualsOperator(Component, SwitchFunc)                      \
  Component::Vector3IndexProxy &Component::Vector3Proxy::operator=(            \
      const Vector3 &v) {                                                      \
    if (m_EntityID == 0)                                                       \
      return *this;                                                            \
    Vector3 vT = v;                                                            \
    SwitchFunc;                                                                \
    return *this                                                               \
  }

#define Vector2IndexProxyDefinition()                                          \
  struct Vector2IndexProxy {                                                   \
  public:                                                                      \
    Vector2IndexProxy(const uint64_t &entityID, const int &index,              \
                      const Field &field)                                      \
        : m_EntityID(entityID), m_Index(index), m_Field(field) {}              \
    operator float();                                                          \
    Vector2IndexProxy &operator=(const float &f);                              \
                                                                               \
  private:                                                                     \
    uint64_t m_EntityID;                                                       \
    int m_Index;                                                               \
    Field m_Field;                                                             \
  };

#define Vector2IndexProxyFloatOperator(Component, SwitchFunc)                  \
  Component::Vector2IndexProxy::operator float() {                             \
    if (m_EntityID == 0)                                                       \
      return 0;                                                                \
    float f;                                                                   \
    SwitchFunc;                                                                \
    return f;                                                                  \
  }

#define Vector2IndexProxyEqualsOperator(Component, SwitchFunc)                 \
  Component::Vector2IndexProxy &Component::Vector2IndexProxy::operator=(       \
      const float &f) {                                                        \
    if (m_EntityID == 0)                                                       \
      return *this;                                                            \
    float fT = f;                                                              \
    SwitchFunc return *this                                                    \
  }

#define Vector2ProxyDefinition()                                               \
  struct Vector2Proxy {                                                        \
  public:                                                                      \
    Vector2Proxy(const uint64_t &entityID, const Field &field)                 \
        : m_EntityID(entityID), m_Field(field) {}                              \
    operator Vector2() const;                                                  \
    Vector2Proxy &operator=(const Vector2 &v);                                 \
    Vector2IndexProxy x() {                                                    \
      return Vector2IndexProxy(m_EntityID, 0, m_Field);                        \
    }                                                                          \
    Vector2IndexProxy y() {                                                    \
      return Vector2IndexProxy(m_EntityID, 1, m_Field);                        \
    }                                                                          \
                                                                               \
  private:                                                                     \
    uint64_t m_EntityID;                                                       \
    Field m_Field;                                                             \
  };

#define Vector2ProxyVector2Operator(Component, SwitchFunc)                     \
  Component::Vector2Proxy::operator Vector2() {                                \
    if (m_EntityID == 0)                                                       \
      return 0;                                                                \
    Vector2 v;                                                                 \
    SwitchFunc;                                                                \
    return v;                                                                  \
  }

#define Vector2ProxyEqualsOperator(Component, SwitchFunc)                      \
  Component::Vector2IndexProxy &Component::Vector2Proxy::operator=(            \
      const Vector2 &v) {                                                      \
    if (m_EntityID == 0)                                                       \
      return *this;                                                            \
    Vector2 vT = v;                                                            \
    SwitchFunc;                                                                \
    return *this                                                               \
  }

#define FloatProxyDefinition()                                                 \
  struct FloatProxy {                                                          \
  public:                                                                      \
    FloatProxy(const uint64_t &entityID, const Field &field)                   \
        : m_EntityID(entityID), m_Field(field) {}                              \
    operator float() const;                                                    \
    FloatProxy &operator=(const float &f);                                     \
                                                                               \
  private:                                                                     \
    uint64_t m_EntityID;                                                       \
    Field m_Field;                                                             \
  };
