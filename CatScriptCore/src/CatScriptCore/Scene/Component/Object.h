#pragma once

#include "Layer.h"
#include "Tag.h"
#include "Transform.h"

#include <cstdint>

namespace CatRuntime {

class Object {
protected:
  uint64_t m_EntityID = 0;

public:
  Object() = default;
  Object(uint64_t entityID) : m_EntityID(entityID) {}

  Object FindObjectByName(const std::string &name);

  Transform transform() { return Transform{m_EntityID}; }

  Tag tag() { return Tag{m_EntityID}; }

  Layer layer() { return Layer{m_EntityID}; }

  template <typename T> bool HasComponent();
  template <typename T> T GetComponent();
public:

  template <typename T>
  bool HasScript() {
    std::string scriptName = typeid(T).name();
    return Object_HasScript(m_EntityID, scriptName.c_str());
  }

  template <typename T> T *GetScript() {
    if (!HasScript<T>())
      return nullptr;

    const char *scriptName = typeid(T).name();
    void *rawMemory = Object_GetScript(m_EntityID, scriptName);
    return reinterpret_cast<T *>(rawMemory);
  }
};
} // namespace CatRuntime
