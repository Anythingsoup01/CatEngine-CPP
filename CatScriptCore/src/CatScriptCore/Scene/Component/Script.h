#pragma once

#include <cstdint>
namespace CatRuntime {

struct Script {
protected:
  uint64_t m_EntityID;

public:
  Script(const uint64_t &entityID) : m_EntityID(entityID) {}
};

}
