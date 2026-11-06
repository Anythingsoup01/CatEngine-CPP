#include "Object.h"
#include "CatScriptCore/InternalCalls/InternalCalls.h"

#include "AllComponents.h"

#include <iostream>

#define Internal_Object_HasComponent(entityID, Type)                           \
  Object_HasComponent(entityID, #Type)

namespace CatRuntime {

// HAS COMPONENT OVERLOADS

template <typename T> bool Object::HasComponent() { return false; }

#define X(Type)                                                                \
  template <> bool Object::HasComponent<Type>() {                              \
    return Internal_Object_HasComponent(m_EntityID, Type);                     \
  }
ENGINE_COMPONENT_LIST(X)
#undef X

// GET COMPONENT OVERLOADS
template <typename T> T Object::GetComponent() { return T{0}; };

#define X(Type)                                                                \
  template <> Type Object::GetComponent<Type>() {                              \
    if (!HasComponent<Type>()) {                                               \
      return Type{0};                                                          \
    }                                                                          \
    return Type{m_EntityID};                                                   \
  }
ENGINE_COMPONENT_LIST(X)
#undef X

} // namespace CatRuntime
