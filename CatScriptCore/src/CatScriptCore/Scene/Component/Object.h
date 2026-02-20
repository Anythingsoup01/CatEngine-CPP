#pragma once

#include "Layer.h"
#include "Tag.h"
#include "Transform.h"
#include <cstdint>

#include <iostream>

namespace CatRuntime {

class Object {
protected:
  uint64_t m_EntityID = 0;
  float m_Control = 0.0f;

public:
  Object() = default;
  Object(uint64_t entityID) : m_EntityID(entityID) {}

  Transform transform() { return Transform{m_EntityID}; }

  Tag tag() { return Tag{m_EntityID}; }

  Layer layer() { return Layer{m_EntityID}; }

  template <typename T> bool HasComponent();

  template <typename T> T GetComponent();
};
} // namespace CatRuntime
