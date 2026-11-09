#pragma once

#include <cstdint>
namespace CatRuntime {

struct Relationship {
protected:
  uint64_t m_EntityID;
public:
  Relationship(const uint64_t entityID): m_EntityID(entityID) {}
};

}
